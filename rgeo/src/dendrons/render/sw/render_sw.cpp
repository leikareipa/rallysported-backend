/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED software renderer.
 *
 */

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include "../../../core/geometry/geometry_sw.h"
#include "../../../core/palette.h"
#include "../../../core/display.h"
#include "../../../core/render.h"
#include "render_sw.h"

// The surface we'll render onto.
static frame_buffer_s FRAME_BUFFER;

void kr_clear_frame(void)
{
    for (uint i = 0; i < (FRAME_BUFFER.r.w * FRAME_BUFFER.r.h); i++)
    {
        FRAME_BUFFER.canvas[i].r = 0;
        FRAME_BUFFER.canvas[i].g = 0;
        FRAME_BUFFER.canvas[i].b = 0;
        FRAME_BUFFER.canvas[i].a = ALPHA_FULLY_OPAQUE;
    }

    return;
}

void resize_frame_buffer(const resolution_s &r)
{
    k_assert(r.bpp == 32, "Was asked to initialize a frame buffer with an invalid bit depth (expected 32).");

    free(FRAME_BUFFER.canvas);

    FRAME_BUFFER.r = r;
    FRAME_BUFFER.canvas = (color_bgra_s*)malloc(r.w * r.h * sizeof(color_bgra_s));
    k_assert(FRAME_BUFFER.canvas != NULL, "Failed to allocate memory for the framebuffer.");

    kr_clear_frame();

    return;
}

void kr_depth_sort_mesh(std::vector<triangle_s> *const mesh)
{
    std::sort(mesh->begin(), mesh->end(), [](const triangle_s &t1, const triangle_s &t2)
                                          {
                                              const i32 d1 = (t1.v[0].z + t1.v[1].z + t1.v[2].z);
                                              const i32 d2 = (t2.v[0].z + t2.v[1].z + t2.v[2].z);
                                              return d1 > d2;
                                          });

    return;
}

void kr_set_depth_testing_enabled(const bool enabled)
{
    /// Not used by the sw renderer.

    (void)enabled;

    return;
}

const frame_buffer_s* kr_framebuffer_ptr(void)
{
    return &FRAME_BUFFER;
}

// Initialize the renderer for an image of width x height x depth. This
// function must be called before any rendering is attempted.
//
const frame_buffer_s* kr_acquire_renderer(void)
{
    const resolution_s r = kd_display_resolution();

    // This function is only supposed to be called once, at program launch when
    // the canvas is still uninitialized.
    k_assert(FRAME_BUFFER.canvas == NULL,
             "The frame buffer canvas must be uninitialized before acquiring the renderer.");

    k_assert(r.bpp == 32,
             "The software renderer requires a 32-bit display.");

    resize_frame_buffer(r);

    FRAME_BUFFER.palette = kpal_current_palette_ptr();

    rs_init_tri_filler(&FRAME_BUFFER);

    return kr_framebuffer_ptr();
}

void kr_release_renderer()
{
    free(FRAME_BUFFER.canvas);

    return;
}

/// Temp hack. Gets it done for now but needs to be cleaned up later.
/*
 * Bresenham line code originally written by Dmitry V. Sokolov
 * (https://github.com/ssloy/tinyrenderer), WITH HEAVY KLUDGE MODIFICATIONS
 * here by Tarpeeksi Hyvae Soft. This does not represent the quality of Mr.
 * Sokolov's original code.
 *
 *      FULL ATTRIBUTION:
 *      Tiny Renderer, https://github.com/ssloy/tinyrenderer
 *      Copyright Dmitry V. Sokolov
 *
 *      This software is provided 'as-is', without any express or implied warranty.
 *      In no event will the authors be held liable for any damages arising from the use of this software.
 *      Permission is granted to anyone to use this software for any purpose,
 *      including commercial applications, and to alter it and redistribute it freely,
 *      subject to the following restrictions:
 *
 *      1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 *      2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 *      3. This notice may not be removed or altered from any source distribution.
 *
 */
static void bresenham_line(const vertex4_s *const _p1, const vertex4_s *const _p2, const palette_idx_t color)
{
    vertex4_s p1 = *_p1;
    vertex4_s p2 = *_p2;

    /// Temp hack.
    if (p1.x < 0)
    {
        p1.x = 0;
    }
    if (p2.x < 0)
    {
        p2.x = 0;
    }
    if (p1.x >= (int)FRAME_BUFFER.r.w)
    {
        p1.x = FRAME_BUFFER.r.w-1;
    }
    if (p2.x >= (int)FRAME_BUFFER.r.w)
    {
        p2.x = FRAME_BUFFER.r.w-1;
    }
    if (p1.y < 0)
    {
        p1.y = 0;
    }
    if (p2.y < 0)
    {
        p2.y = 0;
    }
    if (p1.y >= (int)FRAME_BUFFER.r.h)
    {
        p1.y = FRAME_BUFFER.r.h-1;
    }
    if (p2.y >= (int)FRAME_BUFFER.r.h)
    {
        p2.y = FRAME_BUFFER.r.h-1;
    }

    bool steep = false;

    if (abs(p1.x-p2.x) < abs(p1.y-p2.y))
    {
        std::swap(p1.x, p1.y);
        std::swap(p2.x, p2.y);

        steep = true;
    }

    if (p1.x > p2.x)
    {
        std::swap(p1.x, p2.x);
        std::swap(p1.y, p2.y);
    }

    int dx = p2.x-p1.x;
    int dy = p2.y-p1.y;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = p1.y;

    int a = (p2.y > p1.y? 1 : -1);
    int b = dx * 2;

    if (steep)
    {
        for (int x = p1.x; x <= p2.x; x++)
        {
            //if ((x + y) % 2 == 0)
            {
                FRAME_BUFFER.canvas[y + x * FRAME_BUFFER.r.w].r = color;
                FRAME_BUFFER.canvas[y + x * FRAME_BUFFER.r.w].g = color;
                FRAME_BUFFER.canvas[y + x * FRAME_BUFFER.r.w].b = color;
                FRAME_BUFFER.canvas[y + x * FRAME_BUFFER.r.w].a = ALPHA_FULLY_OPAQUE;

                if ((y-1) >= 0) /// Temp hack. Repeat for a thicker line.
                {
                    FRAME_BUFFER.canvas[(y-1) + x * FRAME_BUFFER.r.w].r = color;
                    FRAME_BUFFER.canvas[(y-1) + x * FRAME_BUFFER.r.w].g = color;
                    FRAME_BUFFER.canvas[(y-1) + x * FRAME_BUFFER.r.w].b = color;
                    FRAME_BUFFER.canvas[(y-1) + x * FRAME_BUFFER.r.w].a = ALPHA_FULLY_OPAQUE;
                }
            }

            error2 += derror2;
            if (error2 > dx)
            {
                y += a;
                error2 -= b;
            }
        }
    }
    else
    {
        for (int x = p1.x; x <= p2.x; x++)
        {
            //if (y >= 0 && y < SCREEN_HEIGHT)
          //  if ((x + y) % 2 == 0)
            {
                FRAME_BUFFER.canvas[x + y * FRAME_BUFFER.r.w].r = color;
                FRAME_BUFFER.canvas[x + y * FRAME_BUFFER.r.w].g = color;
                FRAME_BUFFER.canvas[x + y * FRAME_BUFFER.r.w].b = color;
                FRAME_BUFFER.canvas[x + y * FRAME_BUFFER.r.w].a = ALPHA_FULLY_OPAQUE;

                if ((y-1) >= 0) /// Temp hack. Repeat for a thicker line.
                {
                    FRAME_BUFFER.canvas[x + (y-1) * FRAME_BUFFER.r.w].r = color;
                    FRAME_BUFFER.canvas[x + (y-1) * FRAME_BUFFER.r.w].g = color;
                    FRAME_BUFFER.canvas[x + (y-1) * FRAME_BUFFER.r.w].b = color;
                    FRAME_BUFFER.canvas[x + (y-1) * FRAME_BUFFER.r.w].a = ALPHA_FULLY_OPAQUE;
                }
            }

            error2 += derror2;
            if (error2 > dx)
            {
                y += a;
                error2 -= b;
            }
        }
    }

    return;
}

void kr_rasterize_mesh(const std::vector<triangle_s> &scene, const bool wireframe)
{
    for (const auto &t: scene)
    {
        rs_fill_tri(&t);

        // Draw the ground (and only the ground) with a wireframe.
        if (wireframe &&
            t.interact.type == INTERACTIBLE_GROUND)
        {
            // We want wireframes to show as quads, not triangles, so only draw
            // those edges.
            if (t.interact.params[INTERACT_PARAM_GROUND_IS_MAIN])
            {
                bresenham_line(&t.v[0], &t.v[1], 0);
                bresenham_line(&t.v[1], &t.v[2], 0);
            }
            else
            {
                bresenham_line(&t.v[1], &t.v[2], 0);
                bresenham_line(&t.v[2], &t.v[0], 0);
            }
        }
    }

    return;
}
