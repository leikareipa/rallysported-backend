/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED software renderer: triangle filler.
 *
 * Takes in a triangle, and rasterizes it into the framebuffer.
 *
 */

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include "../../../core/geometry/geometry_sw.h"
#include "../../../core/display.h"
#include "../../../core/fixedpt.h"
#include "../../../core/palette.h"
#include "../../../core/texture.h"
#include "../../../core/render.h"
#include "render_sw.h"

/*
 * TODOS:
 *
 * - there's a bug in the depth fog implementation where the farthest polygons are
 *   fully lit instead of fully dark.
 *
 */

#define DEPTH_FOG_ENABLED
static u8 DEPTH_FOG_LEVEL = 0;    // The palette index we add to the current pixel color to achieve the correct fog shading.

// The surface we'll render to.
static frame_buffer_s *FRAME_BUFFER;

// The triangle we'recurrently filling in. (Assumes single-threaded execution.)
static const triangle_s *TRIANGLE;

void rs_init_tri_filler(frame_buffer_s *const frameBuffer)
{
    FRAME_BUFFER = frameBuffer;

    return;
}

// Split the triangle into two straight-based triangles, one pointing up and one pointing down.
// (Split algo from Bastian Molkenthin's www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html.)
//
static void rs_split_tri(vertex4_s *split,
                         const vertex4_s *high, const vertex4_s *mid, const vertex4_s *low)
{
    real splitRatio = (mid->y - high->y) / (real)(low->y - high->y);

    split->x = high->x + ((low->x - high->x) * splitRatio);
    split->y = mid->y;
    split->uv[0] = LERP(high->uv[0], low->uv[0], splitRatio);
    split->uv[1] = LERP(high->uv[1], low->uv[1], splitRatio);

    return;
}

// 'High' here means low y, where y = 0 is the top of the screen.
//
static void rs_sort_tri_verts(const vertex4_s **high, const vertex4_s **mid, const vertex4_s **low)
{
    if ((*low)->y < (*mid)->y)
    {
        std::swap(*low, *mid);
    }
    if ((*mid)->y < (*high)->y)
    {
        std::swap(*mid, *high);
    }
    if ((*low)->y < (*mid)->y)
    {
        std::swap(*low, *mid);
    }

    return;
}

// Calculates a depth fading level for the current triangle. This assumes that
// we're using an indexed, 256-color palette, with a certain number of primary
// colors and then progressively darkened repetitions of those for the rest of
// the palette. The fading level is then the xth repetition.
//
/// TODO. Has a bug where the farthest, should-be darkest polys are actually fully lit.
static void precalc_depth_fog(void)
{
    #ifdef DEPTH_FOG_ENABLED
        static const uint drawDistance = 4720*3;
        static const uint fadeDist = 750*3;          // The width of the fading band.
        const uint residual = drawDistance - fadeDist;
        const real depth = (TRIANGLE->v[0].w + TRIANGLE->v[1].w + TRIANGLE->v[2].w);
        if (depth >= (drawDistance - 40))   // Maximum fadeout at the far end.
        {
            DEPTH_FOG_LEVEL = kpal_num_primary_colors();
        }
        else if (depth > residual)          // Inside the fading band.
        {
            real m = (1 - ((1.0 / fadeDist) * (depth - residual)));

            CLAMP_VALUE(m, 0, 1);

            m *= (kpal_num_shade_levels() - 1);
            if (m >= kpal_num_shade_levels())
            {
                m = kpal_num_shade_levels() - 1;
            }

            // We're working with indexed color, so the shaded version of this
            // color is found by taking the current index and adding to it the
            // index of the shaded version.
            DEPTH_FOG_LEVEL = (floor(m) * kpal_num_primary_colors());
        }
        else    // No fading.
        {
            DEPTH_FOG_LEVEL = 0;
        }
    #endif

    return;
}

static void rs_fill_tri_row(const uint y,
                            int startX, int endX,
                            real leftU, real leftV,
                            real rightU, real rightV)
{
    const real uDelta = ((rightU - leftU)) / real((endX - startX) + 1); // Amount by which to change the u,v coordinates each pixel on the row.
    const real vDelta = ((rightV - leftV)) / real((endX - startX) + 1);

    // Bounds-check, clip against the screen.
    if (startX < 0)
    {
        // Move the parameters accordingly.
        const uint diff = abs(startX);
        leftU += uDelta * diff;
        leftV += vDelta * diff;

        startX = 0;
    }
    if (endX >= i32(FRAME_BUFFER->r.w))
    {
        endX = (FRAME_BUFFER->r.w - 1);
    }

    if (endX < startX)
    {
        return;
    }

    uint screenPixIdx = startX + y * FRAME_BUFFER->r.w;

    // Solid fill.
    if (TRIANGLE->texturePtr == NULL ||
        TRIANGLE->texturePtr->pixels.is_null())
    {
        const color_rgb_s c = FRAME_BUFFER->palette[TRIANGLE->baseColor + DEPTH_FOG_LEVEL];
        const uint width = ((endX - startX) + 1);

        for (uint i = 0; i < width; i++)
        {
            FRAME_BUFFER->canvas[screenPixIdx].r = c.r;
            FRAME_BUFFER->canvas[screenPixIdx].g = c.g;
            FRAME_BUFFER->canvas[screenPixIdx].b = c.b;
            // FRAME_BUFFER->canvas[screenIdx + i].a = ALPHA_FULLY_OPAQUE;

            screenPixIdx++;
        }
    }
    // Textured fill.
    else
    {
        for (int x = startX; x <= endX; x++)
        {
            const u8 color = TRIANGLE->texturePtr->pixels[(int)leftU + (int)leftV * TRIANGLE->texturePtr->width()];

            // Draw the pixel unless it's see-through.
            if (!(TRIANGLE->baseColor == TEXTURE_ALPHA_ENABLED && color == 0))
            {
                const color_rgb_s c = FRAME_BUFFER->palette[color + DEPTH_FOG_LEVEL];
                FRAME_BUFFER->canvas[screenPixIdx].r = c.r;
                FRAME_BUFFER->canvas[screenPixIdx].g = c.g;
                FRAME_BUFFER->canvas[screenPixIdx].b = c.b;
               // FRAME_BUFFER->canvas[screenIdx].a = ALPHA_FULLY_OPAQUE;
            }

            leftU += uDelta;
            leftV += vDelta;
            screenPixIdx++;
        }
    }

    return;
}

// Pixel-fills the corresponding triangle on screen formed by the three given vertices.
// Expects the vertices to be in screen space, and that the base vertices are level on
// the y axis (you'd first split your triangle along y, then submit the two pieces to
// this function individually).
//
static void fill_split_triangle(const vertex4_s *peak, const vertex4_s *base1, const vertex4_s *base2)
{
    int startRow, endRow;                   // The top and bottom y coordinates of the triangle.
    int triHeight;
    bool isDownTri = false;                 // Whether the triangle peak is above or below the base, in screen space.

    // We'll fill the triangle in row by row, adjusting the row parameters by
    // a delta per every time we move down one row.
    decltype(peak->x) pixLeft, pixRight, dPixLeft, dPixRight;   // A row of pixels defined by its left and right extrema; and deltas for that row.
    real leftU, leftV, rightU, rightV;      // Texture coordinates for the pixel row.
    real dLeftU, dLeftV, dRightU, dRightV;  // Deltas for texture coordinates.

    const vertex4_s *leftVert, *rightVert;

    k_assert((base1->y == base2->y),
             "The software triangle filler was given a malformed triangle. Expected a flat base.");

    // Figure out which corner of the base is on the left/right in screen space.
    leftVert = base2;
    rightVert = base1;
    if (base1->x < base2->x)
    {
        leftVert = base1;
        rightVert = base2;
    }

    startRow = peak->y;
    endRow = base1->y;

    // Detect whether the triangle is the top or bottom half of the split.
    if (startRow > endRow)
    {
        const int temp = startRow;
        startRow = endRow;
        endRow = temp;

        // Don't draw the base row twice; i.e. skip it for the down-triangle.
        startRow++;

        isDownTri = 1;
    }
    // If the triangle is very thin vertically, don't bother drawing it.
    else if ((startRow == endRow) ||
             (endRow - startRow) <= 0)
    {
        return;
    }

    triHeight = endRow - startRow;

    // Establish row parameters.
    if (isDownTri)
    {
        pixLeft = FIXEDP(leftVert->x);
        pixRight = FIXEDP(rightVert->x);
        dPixLeft = FIXEDP(peak->x - leftVert->x) / (triHeight + 1);
        dPixRight = FIXEDP(peak->x - rightVert->x) / (triHeight + 1);

        leftU = (leftVert->uv[0]);
        leftV = (leftVert->uv[1]);
        rightU = (rightVert->uv[0]);
        rightV = (rightVert->uv[1]);
        dLeftU = (peak->uv[0] - leftVert->uv[0]) / real(triHeight + 1);
        dLeftV = (peak->uv[1] - leftVert->uv[1]) / real(triHeight + 1);
        dRightU = (peak->uv[0] - rightVert->uv[0]) / real(triHeight + 1);
        dRightV = (peak->uv[1] - rightVert->uv[1]) / real(triHeight + 1);
    }
    else
    {
        pixLeft = FIXEDP(peak->x);
        pixRight = FIXEDP(peak->x);
        dPixLeft = FIXEDP(leftVert->x - peak->x) / (triHeight + 1);
        dPixRight = FIXEDP(rightVert->x - peak->x) / (triHeight + 1);

        leftU = (peak->uv[0]);
        leftV = (peak->uv[1]);
        rightU = (peak->uv[0]);
        rightV = (peak->uv[1]);
        dLeftU = (leftVert->uv[0] - peak->uv[0]) / real(triHeight + 1);
        dLeftV = (leftVert->uv[1] - peak->uv[1]) / real(triHeight + 1);
        dRightU = (rightVert->uv[0] - peak->uv[0]) / real(triHeight + 1);
        dRightV = (rightVert->uv[1] - peak->uv[1]) / real(triHeight + 1);
    }

    #define INCREMENT_PARAMS(steps)  pixLeft  += dPixLeft  * steps;\
                                     pixRight += dPixRight * steps;\
                                     leftU    += dLeftU    * steps;\
                                     leftV    += dLeftV    * steps;\
                                     rightU   += dRightU   * steps;\
                                     rightV   += dRightV   * steps;

    // Bounds-check to make sure we're not going to draw outside of the screen area
    // horizontally.
    if (startRow < 0)
    {
        const uint steps = abs(startRow);
        INCREMENT_PARAMS(steps)

        startRow = 0;
    }
    if (endRow >= int(FRAME_BUFFER->r.h))
    {
        endRow = (FRAME_BUFFER->r.h - 1);
    }

    // Iterate over each y row in the triangle, on each row filling in a horizontal line between
    // the left and right edge of the triangle.
    for (int y = startRow; y <= endRow; y++)
    {
        INCREMENT_PARAMS(1)

        rs_fill_tri_row(y, UN_FIXEDP(pixLeft), UN_FIXEDP(pixRight), leftU, leftV, rightU, rightV);
    }

    #undef INCREMENT_PARAMS

    return;
}

void rs_fill_tri(const triangle_s *const t)
{
    vertex4_s split;
    const vertex4_s *high = &t->v[0];
    const vertex4_s *mid = &t->v[1];
    const vertex4_s *low = &t->v[2];

    TRIANGLE = t;

    precalc_depth_fog();

    rs_sort_tri_verts(&high, &mid, &low);

    // Split the triangle and draw it on screen in two parts.
    rs_split_tri(&split, high, mid, low);
    fill_split_triangle(high, mid, &split); // Up triangle.
    fill_split_triangle(low, mid, &split);  // Down triangle.

    return;
}


