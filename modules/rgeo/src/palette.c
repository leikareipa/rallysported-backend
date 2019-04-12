/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 */

#include "renderer.h"
#include "common.h"
#include "file.h"

/* For outp().*/
#if __WATCOMC__
    #include <conio.h>
#elif __DMC__
    #include <dos.h>
#elif __GNUC__
    /* Assume this is a modern version of GCC that should just ignore the DOS stuff.*/
    #define outp(x,y) (void)(x); (void)(y);
#endif

/* How many palettes we expect to find in RALLYE.EXE.*/
#define NUM_PALETTES 4

/* Each palette in VGA mode 13h has 256 colors, but Rally-Sport only uses 32
 * primary colors in each of its palettes; the rest are shaded versions of
 * the primary ones.*/
#define NUM_COLORS_IN_PALETTE 32

/* Loads the given palette (0..3) from RALLYE.EXE, and assigns it as the current
 * display palette.*/
void kpal_apply_palette(const uint paletteIdx)
{
    k_assert((paletteIdx < NUM_PALETTES), "Attempting to access palettes out of bounds.");

    k_assert((kr_current_video_mode() == 0x13), "Can only apply a palette while in VGA mode 13h.");

    {
        uint i = 0;
        u8 palette[NUM_COLORS_IN_PALETTE * 3];
        const file_handle rallyeExeHandle = kf_open_file("RALLYE.EXE", "rb");
        const u32 paletteExeOffset = (0x202d6 + (paletteIdx * 3 * NUM_COLORS_IN_PALETTE));

        kf_set_cursor(paletteExeOffset, rallyeExeHandle);
        kf_read_bytes(palette, NUM_ELEMENTS(palette), rallyeExeHandle);

        for (i = 0; i < NUM_COLORS_IN_PALETTE; i++)
        {
            outp(0x03c8, i);
            outp(0x03c9, palette[i * 3 + 0]);
            outp(0x03c9, palette[i * 3 + 1]);
            outp(0x03c9, palette[i * 3 + 2]);
        }

        kf_close_file(rallyeExeHandle);
    }

    return;
}
