/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED user interface: graphical arrangements
 *
 */

#include <unordered_map>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "../../core/texture.h"
#include "../../core/palette.h"
#include "../../core/display.h"
#include "../../core/camera.h"
#include "../../core/maasto.h"
#include "../../core/types.h"
#include "../../core/palat.h"
#include "../../core/text.h"
#include "../../core/ui.h"

/*
 * TODOS:
 *
 * - this unit needs some cleanup overall.
 *
 */

// Cursor graphics.
#include "mouse_cursor.inc"
static cursor_e CURRENT_CURSOR = cursor_e::CURSOR_ARROW;
const std::unordered_map<std::underlying_type<cursor_e>::type, texture_c> *CURSORS;

// How many thumbnails the PALAT pane has on the x and y.
const uint PALAT_PANE_NUM_X = 11;
const uint PALAT_PANE_NUM_Y = 23;

// A texture used to indicate on the minimap where the camera is located.
static texture_c MINIMAP_RECTANGLE_TEXTURE;

static texture_c PALAT_PANE_TEXTURE;
static texture_c PALETTE_SWATCH_TEXTURE;

uint kuil_palat_pane_num_x(void)
{
    return PALAT_PANE_NUM_X;
}

uint kuil_palat_pane_num_y(void)
{
    return PALAT_PANE_NUM_Y;
}

// Returns a list of triangles to make up a rectangle of the given dimensions
// positioned at the given screen coordinates.
void add_screen_space_rect(std::vector<triangle_s> *const scene,
                           const uint x, const uint y,
                           const uint w, const uint h,
                           const texture_c *tex, const bool alpha,
                           const bool flipTexture,
                           const interactible_type_e interaction = INTERACTIBLE_IGNORE)
{
    triangle_s t;

    t.interact.type = interaction;

    t.v[0].w = 1;
    t.v[1].w = 1;
    t.v[2].w = 1;

    t.texturePtr = tex;
    t.baseColor = alpha? TEXTURE_ALPHA_ENABLED : 0;

    // First triangle.
    t.v[0].x = x + 0;
    t.v[0].y = y + 0;
    t.v[0].z = 0;
    t.v[1].x = x + 0;
    t.v[1].y = y + h;
    t.v[1].z = 0;
    t.v[2].x = x + w;
    t.v[2].y = y + h;
    t.v[2].z = 0;
    if (tex != NULL)
    {
        t.v[0].uv[0] = 0;
        t.v[0].uv[1] = 0;
        t.v[1].uv[0] = 0;
        t.v[1].uv[1] = tex->height_unpadded()-0.1;
        t.v[2].uv[0] = tex->width_unpadded()-0.1;
        t.v[2].uv[1] = tex->height_unpadded()-0.1;

        if (flipTexture)
        {
            t.v[0].uv[0] = 0;
            t.v[0].uv[1] = tex->height_unpadded()-0.1;
            t.v[1].uv[0] = 0;
            t.v[1].uv[1] = 0;
            t.v[2].uv[0] = tex->width_unpadded()-0.1;
            t.v[2].uv[1] = 0;
        }
    }
    scene->push_back(t);

    // Second triangle.
    t.v[0].x = x + w;
    t.v[0].y = y + h;
    t.v[0].z = 0;
    t.v[1].x = x + w;
    t.v[1].y = y + 0;
    t.v[1].z = 0;
    t.v[2].x = x + 0;
    t.v[2].y = y + 0;
    t.v[2].z = 0;
    if (tex != NULL)
    {
        t.v[0].uv[0] = tex->width_unpadded()-0.1;
        t.v[0].uv[1] = tex->height_unpadded()-0.1;
        t.v[1].uv[0] = tex->width_unpadded()-0.1;
        t.v[1].uv[1] = 0;
        t.v[2].uv[0] = 0;
        t.v[2].uv[1] = 0;

        if (flipTexture)
        {
            t.v[0].uv[0] = tex->width_unpadded()-0.1;
            t.v[0].uv[1] = 0;
            t.v[1].uv[0] = tex->width_unpadded()-0.1;
            t.v[1].uv[1] = tex->height_unpadded()-0.1;
            t.v[2].uv[0] = 0;
            t.v[2].uv[1] = tex->height_unpadded()-0.1;
        }
    }
    scene->push_back(t);

    return;
}

void kuil_render_string(const char *const str,
                      uint x, uint y,
                      std::vector<triangle_s> *const scene,
                      const bool alpha)
{
    const real resMulX = NATIVE_RES_MUL_X;
    const real resMulY = NATIVE_RES_MUL_Y;
    const uint strLen = strlen(str);
    const uint strPixelLen = (ktext_font_width()/2 + 1) * strLen;

    // We expect the coordinates to be given relative to the game's internal
    // resolution (320 x 200). Adjust them for the actual display.
    x *= resMulX;
    y *= resMulY;

    // Draw a black bar under the text, to hide whatever's underneath.
    if (!alpha)
    {
        add_screen_space_rect(scene,
                              x-2*resMulX, y,
                              (strPixelLen + 2) * resMulX, (ktext_font_height()-1) * resMulY,
                              NULL, false, false);
    }

    // Insert each character as a texture rectangle into the scene mesh.
    for (uint i = 0; i < strLen; i++)
    {
        add_screen_space_rect(scene,
                              x + ((ktext_font_width()/2 + 1) * i) * resMulX, y,
                              ktext_font_width() * resMulX, ktext_font_height() * resMulY,
                              ktext_character_texture(str[i]), true, false);
    }

    return;
}

static void show_fps(std::vector<triangle_s> *const scene)
{
    uint fps = kd_current_fps();
    if (fps > 999)
    {
        fps = 999;
    }

    static char fpsDisplay[16];
    snprintf(fpsDisplay, NUM_ELEMENTS(fpsDisplay), "FPS:%u", fps);
    kuil_render_string(fpsDisplay, (kui_paint_view_is_open()? 285 : 220), 0, scene);

    return;
}

static void add_current_pala_indicator(std::vector<triangle_s> *const scene)
{
    const uint x = (298 * NATIVE_RES_MUL_X);
    const uint y = (36 * NATIVE_RES_MUL_Y);

    // Black halo around the PALA.
    add_screen_space_rect(scene,
                          (x - NATIVE_RES_MUL_X), (y - NATIVE_RES_MUL_X),
                          (kuil_current_cursor_texture().width() + 2) * NATIVE_RES_MUL_X,
                          (kuil_current_cursor_texture().height() + 2) * NATIVE_RES_MUL_Y,
                          NULL, false, false);

    // The current PALA.
    add_screen_space_rect(scene,
                          x, y,
                          kuil_current_cursor_texture().width() * NATIVE_RES_MUL_X,
                          kuil_current_cursor_texture().height() * NATIVE_RES_MUL_Y,
                          kpalat_pala_ptr(kui_brush_pala_idx(), false), true, true);

    char brushSize[8];
    snprintf(brushSize, NUM_ELEMENTS(brushSize), "*%d", kui_brush_size());
    kuil_render_string(brushSize, 305, 52, scene);

    return;
}

// Draws up the PALAT pane texture according to the current pixel data in the PALA
// textures.
//
void kuil_update_palat_pane_texture(void)
{
    const uint palaWidth = kpalat_pala_width();
    const uint palaHeight = kpalat_pala_height();

    // Draw the thumbnails into the texture.
    uint palaIdx = 0;
    for (uint y = 0; y < PALAT_PANE_NUM_Y; y++)
    {
        for (uint x = 0; x < PALAT_PANE_NUM_X; x++)
        {
            const texture_c *const tex = kpalat_pala_ptr(palaIdx++);

            if (palaIdx >= 252)
            {
                break;
            }

            for (uint py = 0; py < palaHeight; py++)
            {
                for (uint px = 0; px < palaWidth; px++)
                {
                    const uint palaIdx = px + (palaWidth - 1 - py) * palaWidth;
                    const uint texIdx = ((x * palaHeight + px + 0) / 2) + ((y * palaHeight + py + 0) / 2) * PALAT_PANE_TEXTURE.width();

                    PALAT_PANE_TEXTURE.pixels[texIdx] = tex->pixels[palaIdx];
                }
            }
        }
    }

    // Draw a grid over the palas.
    for (unsigned int i = 0; i < PALAT_PANE_NUM_Y * palaHeight/2; i++)
    {
        for (uint x = 0; x < PALAT_PANE_NUM_X; x++)
        {
            PALAT_PANE_TEXTURE.pixels[(x * palaWidth/2+0) + (i+0) * PALAT_PANE_TEXTURE.width()] = 0;
        }
    }
    for (unsigned int i = 0; i < PALAT_PANE_NUM_X * palaWidth/2; i++)
    {
        for (uint y = 0; y < PALAT_PANE_NUM_Y; y++)
        {
           PALAT_PANE_TEXTURE.pixels[(i+0) + (y * palaHeight/2+0) * PALAT_PANE_TEXTURE.width()] = 0;
        }
    }

    if (!PALAT_PANE_TEXTURE.is_padded())
    {
        PALAT_PANE_TEXTURE.pad_to_pot_bounding_square();
    }

    PALAT_PANE_TEXTURE.make_available_to_renderer();  // Make sure any new changes are reflected in the renderer.

    return;
}

void kuil_initialize_ui_layout(void)
{
    // Create a small topdown map of the track.
    {
        kmaasto_generate_topdown_view();

        // Create a texture with solid borders and a see-through hole in the middle,
        // for indicating on the map where the camera is located.
        {
            MINIMAP_RECTANGLE_TEXTURE.initialize({8, 8, 8}, "Minimap camera rectangle texture");
            MINIMAP_RECTANGLE_TEXTURE.isFiltered = false;
            memset(MINIMAP_RECTANGLE_TEXTURE.pixels.ptr(), 0,
                   MINIMAP_RECTANGLE_TEXTURE.pixels.up_to(MINIMAP_RECTANGLE_TEXTURE.width() * MINIMAP_RECTANGLE_TEXTURE.height()));

            // Draw up the texture.
            for (uint i = 0; i < MINIMAP_RECTANGLE_TEXTURE.width(); i++)
            {
                const u8 borderColor = 6;

                // Horizontal borders.
                MINIMAP_RECTANGLE_TEXTURE.pixels[i] = borderColor;
                MINIMAP_RECTANGLE_TEXTURE.pixels[i + (MINIMAP_RECTANGLE_TEXTURE.height() - 1) * MINIMAP_RECTANGLE_TEXTURE.width()] = borderColor;

                // Vertical borders.
                MINIMAP_RECTANGLE_TEXTURE.pixels[i * MINIMAP_RECTANGLE_TEXTURE.width()] = borderColor;
                MINIMAP_RECTANGLE_TEXTURE.pixels[(MINIMAP_RECTANGLE_TEXTURE.width() - 1) + i * MINIMAP_RECTANGLE_TEXTURE.width()] = borderColor;
            }

            MINIMAP_RECTANGLE_TEXTURE.make_available_to_renderer();
        }
    }

    // Create the PALAT pane's texture.
    {
        PALAT_PANE_TEXTURE.initialize({(PALAT_PANE_NUM_X * 8 + 1),
                                       (PALAT_PANE_NUM_Y * 8 + 1), 8}, "PALAT pane texture");
        PALAT_PANE_TEXTURE.isFiltered = false;
        kuil_update_palat_pane_texture();
        PALAT_PANE_TEXTURE.make_available_to_renderer();
    }

    // Create a palette swatch texture for the texture editor.
    {
        const uint swatchSize = 6;
        const uint numColors = kpal_num_primary_colors();

        PALETTE_SWATCH_TEXTURE.initialize({swatchSize, (swatchSize * numColors), 8}, "Palette swatch texture");
        PALETTE_SWATCH_TEXTURE.isFiltered = false;

        // Draw the swatch for each available color.
        for (uint i = 0; i < numColors; i++)
        {
            for (uint y = 0; y < swatchSize; y++)
            {
                const uint startOffs = (y + (swatchSize * i)) * PALETTE_SWATCH_TEXTURE.width();
                memset(&PALETTE_SWATCH_TEXTURE.pixels[startOffs], i,
                       PALETTE_SWATCH_TEXTURE.pixels.up_to(startOffs + swatchSize));
            }
        }

        PALETTE_SWATCH_TEXTURE.pad_to_pot_bounding_square();
        PALETTE_SWATCH_TEXTURE.make_available_to_renderer();
    }

    // Create the program's cursors.
    {
        auto make_cursor_texture = [](const u8 *const pixels)
                                   {
                                        texture_c cursor({CURSOR_WIDTH, CURSOR_HEIGHT, 8}, "Cursor texture");
                                        cursor.isFiltered = false;
                                        memcpy(cursor.pixels.ptr(), pixels, cursor.pixels.up_to(CURSOR_WIDTH * CURSOR_HEIGHT));
                                        cursor.make_available_to_renderer();
                                        return cursor;
                                   };

        static std::unordered_map<std::underlying_type<cursor_e>::type, texture_c> cursors;
        cursors[CURSOR_ARROW]           = make_cursor_texture(CURSOR_IMAGE_ARROW);
        cursors[CURSOR_ARROWSMOOTHING]  = make_cursor_texture(CURSOR_IMAGE_ARROWSMOOTHING);
        cursors[CURSOR_NOTALLOWED]      = make_cursor_texture(CURSOR_IMAGE_NOTALLOWED);
        cursors[CURSOR_HAND]            = make_cursor_texture(CURSOR_IMAGE_HAND);
        cursors[CURSOR_HANDGRAB]        = make_cursor_texture(CURSOR_IMAGE_HANDGRAB);
        cursors[CURSOR_INVISIBLE]       = make_cursor_texture(CURSOR_IMAGE_INVISIBLE);
        CURSORS = &cursors;
    }

    return;
}

void kuil_release_ui_layout(void)
{
    MINIMAP_RECTANGLE_TEXTURE.pixels.release_memory();
    PALAT_PANE_TEXTURE.pixels.release_memory();
    PALETTE_SWATCH_TEXTURE.pixels.release_memory();

    return;
}

static void add_minimap(std::vector<triangle_s> *const scene)
{
    const real nativeXMul = NATIVE_RES_MUL_X;
    const real nativeYMul = NATIVE_RES_MUL_Y;
    const uint x = 256 * nativeXMul;
    const uint y = 0 * nativeYMul;
    const uint sizeMul = 128 / kmaasto_track_width();  /// <- Assumes the track is square.
    const uint camX = (256 + kcamera_tile_position_x()/(2 / sizeMul)) * nativeXMul;
    const uint camY = (0   + kcamera_tile_position_y()/(4 / sizeMul)) * nativeYMul;

    // Black halo around the minimap.
    add_screen_space_rect(scene,
                          x-nativeXMul, y-nativeYMul,
                          (64 + 2) * nativeXMul,
                          (32 + 2) * nativeYMul,
                          NULL, false, false);

    // Draw the minimap itself.
    add_screen_space_rect(scene,
                          x, y,
                          64 * nativeXMul,
                          32 * NATIVE_RES_MUL_Y,
                          kmaasto_track_topdown_texture_ptr(), false, false);

    // Draw a rectangle indicating where the camera is on the minimap.
    add_screen_space_rect(scene,
                          camX-(2 * nativeXMul), camY-(2 * nativeYMul),
                          MINIMAP_RECTANGLE_TEXTURE.width()*2 * nativeXMul * sizeMul,
                          MINIMAP_RECTANGLE_TEXTURE.height() * nativeYMul * sizeMul,
                          &MINIMAP_RECTANGLE_TEXTURE, true, false);

    return;
}

/// Temp hack, so we can share the size of the pane to other units.
uint kuil_palat_pane_width(void)
{
    const real ratio = 1;//1 + ((kd_display_resolution().h / (real)NATIVE_RESOLUTION.h) - floor(kd_display_resolution().h / (real)NATIVE_RESOLUTION.h));
    const uint width = PALAT_PANE_NUM_X * (8*(NATIVE_RES_MUL_X)) * ratio;

    return width;
}
uint kuil_palat_pane_height(void)
{
    const uint height = PALAT_PANE_NUM_Y * (8*(NATIVE_RES_MUL_X));
    ///                                             ^ Defined on purpose for X rather than Y, while sorting out some issues with scaling on various resolutions.

    return height;
}

static void add_palat_pane(std::vector<triangle_s> *const scene)
{
    const real resMulX = NATIVE_RES_MUL_X;
    const real resMulY = NATIVE_RES_MUL_X;  /// Defined on purpose for X rather than Y, for now while sorting out some issues with scaling on various resolutions.
    const real ratio = 1;//1 + ((kd_display_resolution().h / (real)NATIVE_RESOLUTION.h) - floor(kd_display_resolution().h / (real)NATIVE_RESOLUTION.h));
    const uint curPalaIdx = kui_brush_pala_idx();
    const uint palaY = floor(curPalaIdx / real(PALAT_PANE_NUM_X));
    const uint palaX = curPalaIdx - (palaY * PALAT_PANE_NUM_X);

    // Draw the pane.
    add_screen_space_rect(scene,
                          0, 0,
                          kuil_palat_pane_width(),
                          kuil_palat_pane_height(),
                          &PALAT_PANE_TEXTURE, false, false, INTERACTIBLE_PALATPANE);

    // Draw a selection rectangle around the PALA that is currently selected.
    add_screen_space_rect(scene,
                          palaX * (8*(resMulX)) * ratio,
                          palaY * (8*(resMulY)),
                          MINIMAP_RECTANGLE_TEXTURE.width() * NATIVE_RES_MUL_X,
                          MINIMAP_RECTANGLE_TEXTURE.height() * NATIVE_RES_MUL_Y,
                          &MINIMAP_RECTANGLE_TEXTURE, true, false);

    return;
}

static void add_texedit_view(std::vector<triangle_s> *const scene)
{
    static char stringBuf[64];
    const real nativeXMul = NATIVE_RES_MUL_X;
    const real nativeYMul = NATIVE_RES_MUL_Y;
    const uint baseSize = kpalat_pala_width();
    const uint sizeMul = 8;
    const uint numColors = kpal_num_primary_colors();
    const real fontYSkip = kd_display_resolution().h / (real)numColors / nativeYMul;
    const uint posX = (kd_display_resolution().w - (baseSize * sizeMul * nativeXMul) + kuil_palat_pane_width()/2) / 2;
    const uint posY = (kd_display_resolution().h - (baseSize * sizeMul * nativeYMul)) / 2;

    // Draw the PALA we're editing.
    add_screen_space_rect(scene,
                          posX, posY,
                          baseSize * nativeXMul * sizeMul,
                          baseSize * nativeYMul * sizeMul,
                          kpalat_pala_ptr(kui_brush_pala_idx(), false), false, true);

    // Print out the swatch indices.
    for (uint i = 0; i < numColors; i++)
    {
        snprintf(stringBuf, NUM_ELEMENTS(stringBuf), "-%u", i);
        kuil_render_string(stringBuf, 292, (i * fontYSkip), scene, true);
    }

    // Draw the palette swatch.
    add_screen_space_rect(scene,
                          280 * nativeXMul, 0,
                          10 * nativeXMul,
                          kd_display_resolution().h,
                          &PALETTE_SWATCH_TEXTURE, false, false);

    // Highlight the selected color with a symbol next to it.
    kuil_render_string("$", 271, (kui_brush_color() * fontYSkip), scene, true);

    // Print above the edit view the # of the PALA being edited.
    snprintf(stringBuf, NUM_ELEMENTS(stringBuf), "PALA:%u", kui_brush_pala_idx());
    kuil_render_string(stringBuf, posX/nativeXMul, 28, scene);

    return;
}

static void add_paint_view(std::vector<triangle_s> *const scene)
{
    static char stringBuf[64];
    const uint baseSize = 128;
    const uint sizeMul = baseSize / kmaasto_track_width();  /// <- Assumes the track is square.
    const uint posX = (kd_display_resolution().w - (baseSize * NATIVE_RES_MUL_X * 2)) / 2;
    const uint posY = (kd_display_resolution().h - (baseSize * NATIVE_RES_MUL_Y)) / 2;

    add_screen_space_rect(scene,
                          posX, posY,
                          kmaasto_track_width()*2 * NATIVE_RES_MUL_X * sizeMul,
                          kmaasto_track_height() * NATIVE_RES_MUL_Y * sizeMul,
                          kmaasto_track_topdown_texture_ptr(), false, false);

    snprintf(stringBuf, NUM_ELEMENTS(stringBuf), "TRACK SIZE:%u,%u", kmaasto_track_width(), kmaasto_track_height());
    kuil_render_string(stringBuf, 160 + baseSize - (strlen(stringBuf) * ktext_font_unpadded_width()) + 1, 29, scene); // Place the text on the upper right corner of the map.

    return;
}

static void add_rgeo_watermark(std::vector<triangle_s> *const scene)
{
    kuil_render_string("RALLY", 3, 1, scene);
    kuil_render_string("SPORT", 3, 7, scene);

    // The color of the flag symbol shows whether there are unsaved changes.
    if (kui_are_unsaved_changes())
    {
        kuil_render_string("ED& ", 3, 13, scene);
    }
    else
    {
        kuil_render_string("ED% ", 3, 13, scene);
    }

    return;
}

static void add_bottom_info_text(std::vector<triangle_s> *const scene)
{
    static char stringBuf[256];

    if (kui_palat_pane_is_open() &&
        kui_cursor_is_inside_palat_pane())
    {
        if (kui_hovered_pala_idx() <= 250)
        {
            snprintf(stringBuf, NUM_ELEMENTS(stringBuf), "PALA# %3u", kui_hovered_pala_idx());
        }
        else
        {
            snprintf(stringBuf, NUM_ELEMENTS(stringBuf), "PALA# ---");
        }
    }
    else if (kui_current_interaction().type == INTERACTIBLE_GROUND &&
             !kcamera_camera_is_moving())
    {
        const uint palaX = kui_current_interaction().params[INTERACT_PARAM_GROUND_X_GLOBAL];
        const uint palaY = kui_current_interaction().params[INTERACT_PARAM_GROUND_Z_GLOBAL];
        const int height = kmaasto_height_at(palaX, palaY);

        snprintf(stringBuf, NUM_ELEMENTS(stringBuf), "H:%+.3i P:%.3u X,Y:%.3i,%.3i",
                 height,
                 kmaasto_texture_at(palaX, palaY), palaX, palaY);
    }
    else
    {
        snprintf(stringBuf, NUM_ELEMENTS(stringBuf), "H:---- P:--- X,Y:---,---");
    }

    kuil_render_string(stringBuf, 4, 192, scene);

    if (kui_is_real_water_level_enabled() &&
        !kui_texedit_view_is_open()) // <- If the texedit view is open, the string would obsure parts of the interactible UI.
    {
        kuil_render_string("REAL WATER HEIGHT", 233, 192, scene);
    }

    return;
}

void add_mouse_cursor(std::vector<triangle_s> *const scene)
{
    add_screen_space_rect(scene,
                          kui_cursor_pos().x, kui_cursor_pos().y,
                          kuil_current_cursor_texture().width() * NATIVE_RES_MUL_X,
                          kuil_current_cursor_texture().height() * NATIVE_RES_MUL_Y,
                          &kuil_current_cursor_texture(), true, false);

    return;
}

// Based on which view the user currently has open, create the relevant set of
// screen-space triangles to portray for that view its user interface.
//
void kuil_add_user_interface_tri_mesh(std::vector<triangle_s> *const scene)
{
    add_rgeo_watermark(scene);

    if (kui_texedit_view_is_open())
    {
        add_texedit_view(scene);
    }
    else if (kui_paint_view_is_open())
    {
        add_paint_view(scene);
        add_current_pala_indicator(scene);
    }
    else
    {
        add_minimap(scene);
        add_current_pala_indicator(scene);
    }

    if (kui_palat_pane_is_open() ||
        kui_texedit_view_is_open())
    {
        add_palat_pane(scene);
    }

    if (kui_is_fps_display_on())
    {
        show_fps(scene);
    }

    add_bottom_info_text(scene);
    add_mouse_cursor(scene);

    return;
}

void kuil_set_cursor(const cursor_e c)
{
    k_assert(CURSORS->find(c) != CURSORS->end(), "Was asked to set an unknown cursor type.");

    CURRENT_CURSOR = c;

    return;
}

const texture_c& kuil_current_cursor_texture(void)
{
    return CURSORS->at(CURRENT_CURSOR);
}

// Looks at the size of the current display to decide what would be the best resolution
// for the program. Doesn't allow user intervention at the moment. If fullscreen is true,
// will prefer to return a fullscreen-friendly resolution (these are meant primarily for
// old systems, with 4:3 resolutions of 1024 x 768 or less).
//
#if defined(RSED_ON_WIN32)
    #include <windows.h>
#elif defined(RSED_ON_QT)
    #include <QDesktopWidget>
    #include <QApplication>
#endif
resolution_s kuil_ideal_display_resolution(const bool wantFullscreen)
{
    resolution_s displayRes;
    std::vector<resolution_s> knownResolutions;

    uint bpp = 32;
    #if defined(RSED_ON_QT)
        bpp = 32;
    #elif defined(RSED_ON_WIN32)
        bpp = 16;
    #else
        #error "Unknown platform."
    #endif

    if (wantFullscreen)
    {
        knownResolutions.push_back({1920, 1080, bpp});
        knownResolutions.push_back({1366, 768, bpp});
        knownResolutions.push_back({1024, 768, bpp});
        knownResolutions.push_back({1024, 768, bpp});
        knownResolutions.push_back({800, 600, bpp});
        knownResolutions.push_back({640, 480, bpp});

        // Get the current desktop resolution.
        #if defined(RSED_ON_WIN32)
            displayRes.w = GetSystemMetrics(SM_CXSCREEN);
            displayRes.h = GetSystemMetrics(SM_CYSCREEN);
        #elif defined(RSED_ON_QT)
            displayRes.w = QApplication::desktop()->geometry().width();
            displayRes.h = QApplication::desktop()->geometry().height();
        #else
            #error "Unknown platform."
        #endif
    }
    else	// Windowed.
    {
        // These are intended to be integer multiples of 320 x 200, Rally-Sport's
        // native resolution.
        knownResolutions.push_back({1920, 1200, bpp});
        knownResolutions.push_back({1280, 800, bpp});
        knownResolutions.push_back({960, 600, bpp});
        knownResolutions.push_back({640, 400, bpp});
        knownResolutions.push_back({320, 200, bpp});

        // Get the size of the desktop's work area.
        #if defined(RSED_ON_WIN32)
            RECT screen;
            SystemParametersInfo(SPI_GETWORKAREA, 0, &screen, 0);
            displayRes.w = screen.right - screen.left;
            displayRes.h = screen.bottom;
        #elif defined(RSED_ON_QT)
            displayRes.w = QApplication::desktop()->availableGeometry().width();
            displayRes.h = QApplication::desktop()->availableGeometry().height();
        #else
            #error "Unknown platform."
        #endif
    }

    DEBUG(("Searching for the best resolution to match the screen area of %d x %d...", displayRes.w, displayRes.h));

    // Select the largest resolution that fits on the screen.
    for (uint i = 0; i < knownResolutions.size(); i++)
    {
        if (knownResolutions[i].w <= displayRes.w &&
            knownResolutions[i].h <= displayRes.h)
        {
            displayRes = knownResolutions[i];
            goto done;
        }
    }

    ERRORI(("Failed to find a suitable window resolution. Falling back to lowest denominator."));
    displayRes = knownResolutions.back();

    done:
    DEBUG(("Selected %u x %u as the most suitable resolution.", displayRes.w, displayRes.h));
    return displayRes;
}
