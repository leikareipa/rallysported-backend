/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 */

#include "common.h"
#include "file.h"

#define NUM_COLORS_IN_PALETTE 32

/* Each color has three channels, RGB.*/
static u8 palette[NUM_COLORS_IN_PALETTE * 3];

/* Loads the given palette (0..3) from RALLYE.EXE.*/
void kpal_initialize_palette(const uint paletteId)
{
    const file_handle fh = kf_open_file("RALLYE.EXE", "rb");

    /* Get the starting offset in RALLYE.EXE of the first color of the
     * desired palette.*/
    const u32 startOffset = (0x202d6 + (paletteId * 3 * NUM_COLORS_IN_PALETTE));

    kf_jump(startOffset, fh);
    kf_read_bytes(palette, NUM_ELEMENTS(palette), fh);

    kf_close_file(fh);

    return;
}

const u8* kpal_palette(void)
{
    return palette;
}
