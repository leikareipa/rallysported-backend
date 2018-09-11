/*
 * 2017, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED palette
 *
 * Manages access to Rally-Sport's hardcoded VGA mode 13h palettes.
 *
 * Like in Rally-Sport, RallySportED's rendering is based on the VGA mode 13h
 * palette of 256 colors. Even when the framebuffer has e.g. 32-bit color depth,
 * the individual colors will still be drawn from this pool of 256 entries,
 * generally including for effects like distance fog.
 *
 */

#include <cstdlib>
#include "../../core/manifesto.h"
#include "../../core/game_exe.h"
#include "../../core/display.h"
#include "../../core/palette.h"
#include "../../core/common.h"
#include "../../core/memory.h"
#include "../../core/file.h"

static const int NUM_PALETTES = 4;
static int ACTIVE_PALETTE_IDX = -1;             // Which of the palettes is the one currently active. This depends on which track we're editing.
static heap_bytes_s<color_rgb_s> PALETTES[NUM_PALETTES];

static const int PALETTE_SIZE = 256;            // Number of entries in the VGA mode 13h palette.
static const int NUM_COLORS_IN_PALETTE = 32;    // Number of user-definable entries in Rally-Sport's palette.

static const uint NUM_SHADE_LEVELS = 8;         // For each primary color in the palette, how many gradations between it and total black to create.

static_assert(((NUM_SHADE_LEVELS * NUM_COLORS_IN_PALETTE) == PALETTE_SIZE), "Expected primary colors + shading to use the entire palette and no more.");

uint kpal_num_shade_levels(void)
{
    return NUM_SHADE_LEVELS;
}

uint kpal_palette_size(void)
{
    return PALETTE_SIZE;
}

// Loads the game's hardcoded palettes.
//
static void pal_load_palettes_from_executable(void)
{
    const file_handle_t fh = kge_file_handle_rallye_exe();

    // Seek to the start of the palette block.
    kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_PALETTE, 0), fh);

    for (uint p = 0; p < NUM_PALETTES; p++)
    {
        // Read in all 32 primary colors of the palette.
        for (uint i = 0; i < NUM_COLORS_IN_PALETTE; i++)
        {
            color_rgb_s c = {kfile_read_value<u8>(fh),
                             kfile_read_value<u8>(fh),
                             kfile_read_value<u8>(fh)};

            // Create shaded versions of the primary color.
            for (uint s = 1; s < NUM_SHADE_LEVELS; s++)
            {
                const u8 idx = (s * NUM_COLORS_IN_PALETTE) + i;

                const int r = (c.r / NUM_SHADE_LEVELS) * (s - 1);
                const int g = (c.g / NUM_SHADE_LEVELS) * (s - 1);
                const int b = (c.b / NUM_SHADE_LEVELS) * (s - 1);

                PALETTES[p][idx].r = (r * 4); // *4 to convert from the VGA 13h 0-63 range to normal 0-255.
                PALETTES[p][idx].g = (g * 4);
                PALETTES[p][idx].b = (b * 4);
            }

            // Add the primary color itself.
            c.r *= 4;
            c.g *= 4;
            c.b *= 4;
            PALETTES[p][i] = c;
        }
    }

    return;
}

uint kpal_num_palettes(void)
{
    return NUM_PALETTES;
}

uint kpal_num_primary_colors(void)
{
    return NUM_COLORS_IN_PALETTE;
}

void kpal_initialize_palettes()
{
    // Select which palette is active based on which track is loaded.
    switch (kmanif_track_idx())
    {
        case 1: ACTIVE_PALETTE_IDX = 0; break;
        case 2: ACTIVE_PALETTE_IDX = 0; break;
        case 3: ACTIVE_PALETTE_IDX = 0; break;
        case 4: ACTIVE_PALETTE_IDX = 0; break;
        case 5: ACTIVE_PALETTE_IDX = 1; break;
        case 6: ACTIVE_PALETTE_IDX = 0; break;
        case 7: ACTIVE_PALETTE_IDX = 0; break;
        case 8: ACTIVE_PALETTE_IDX = 3; break;

        default: ACTIVE_PALETTE_IDX = 0; break;
    }

    k_assert((ACTIVE_PALETTE_IDX < NUM_PALETTES), "Was asked to initialize a palette out of bounds. Won't do it.");

    for (uint i = 0; i < NUM_PALETTES; i++)
    {
        PALETTES[i].alloc(PALETTE_SIZE, "Palette");
    }

    pal_load_palettes_from_executable();

    return;
}

// Returns a pointer to the beginning of the currently-active palette.
const color_rgb_s* kpal_current_palette_ptr(void)
{
    k_assert(ACTIVE_PALETTE_IDX >= 0, "Active palette index less than 0. Was the palette intialized yet?");
    k_assert(ACTIVE_PALETTE_IDX < NUM_PALETTES, "Palette index out of bounds.");

    return PALETTES[ACTIVE_PALETTE_IDX].ptr();
}

// Returns the RGB equivalent of the given palette color index.
//
color_rgb_s kpal_index_to_rgb(const uint idx)
{
    k_assert(idx < PALETTE_SIZE, "Palette access out of bounds.");

    return PALETTES[ACTIVE_PALETTE_IDX][idx];
}

void kpal_release_palette(void)
{
    for (uint i = 0; i < NUM_PALETTES; i++)
    {
        PALETTES[i].release_memory();
    }

    return;
}
