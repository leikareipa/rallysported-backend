/*
 * 2017, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED PALAT
 *
 * Handles data stored in Rally-Sport's PALAT files. A PALA is a small, 16-by-16-
 * pixel texture used on track tiles - each PALAT file has about 200 of them. The
 * pixels are u8 values indexing a 256-color VGA palette.
 *
 * Call the initializer function, after which you can call the various other ones
 * to access the PALA data.
 *
 */

#include <vector>
#include <cstdlib>
#include <cstring>
#include "../../core/texture.h"
#include "../../core/display.h"
#include "../../core/common.h"
#include "../../core/palat.h"
#include "../../core/file.h"
#include "../../core/main.h"
#include "../../core/ui.h"

// Pixel size.
static const uint PALA_WIDTH = 16;
static const uint PALA_HEIGHT = 16;

static const uint MAX_NUM_PALAS = 252;              // How many PALA textures to load from any given PALAT file.

// Information about the PALAT data's location in the project file.
static file_handle_t FH_PALAT = 0;
static u32 FPOS_PALAT = 0;                          // File offset.
static u32 FLEN_PALAT = 0;                          // Data size.

// Raw PALA pixel data as loaded in from the PALAT file.
static heap_bytes_s<u8> PALAT_DATA;

// Each PALA as an individual texture.
static std::vector<texture_c> PALAT;
static std::vector<texture_c> PALAT_UNFILTERED;     // Same but with texture-filtering disabled.

// When the user requests individual PALA textures out of range, this null one
// may be returned.
static texture_c NULL_PALA;

// Returns a pointer to the data of the PALA of the given index.
//
const texture_c* kpalat_pala_ptr(const uint idx, const bool filtered)
{
    const std::vector<texture_c> &palat = (filtered? PALAT : PALAT_UNFILTERED);

    // The Matrox Mystique doesn't support texture-filtering, so only return
    // unfiltered textures.
    #ifdef RENDERER_IS_MSI
        palat = &PALAT_UNFILTERED;
    #endif

    if (idx >= palat.size())
    {
        k_assert(!NULL_PALA.pixels.is_null(),
                 "The null texture was needed but had not been initialized.");

        return &NULL_PALA;
    }

    return &palat[idx];
}

// Modify the given pixel in the idx'th PALA.
//
void kpalat_set_pala_pixel(const u8 palaIdx, const u8 x, const u8 y, const u8 color)
{
    k_assert(palaIdx < PALAT.size(), "Tried to set the pixels of an out-of-bounds PALA.");
    k_assert(PALAT.size() == PALAT_UNFILTERED.size(), "");  /// Shouldn't happen, but test for it anyway to make sure.

    PALAT[palaIdx].pixels[x + y * PALA_WIDTH] = color;   // The filtered and unfiltered versions have linked pixel data, so this'll edit them both.

    PALAT[palaIdx].make_available_to_renderer();
    PALAT_UNFILTERED[palaIdx].make_available_to_renderer();

    kui_flag_unsaved_changes();

    return;
}

// Overwrite the previous PALAT data on disk with our current stuff.
//
bool kpalat_save_palat(void)
{
    kfile_seek(FPOS_PALAT, FH_PALAT);
    kfile_write_byte_array(PALAT_DATA.ptr(), PALAT_DATA.up_to(FLEN_PALAT), FH_PALAT);
    kfile_flush_file(FH_PALAT);

    return true;
}

// Main point of entry; loads the PALA data from the given file (assumed to be a
// RallySportED project container). Assumes that the file's cursor is positioned
// correctly for this loading to begin without further seeks.
//
void kpalat_initialize_palat(const file_handle_t fileHandle)
{
    const u32 fileSize = kfile_read_value<u32>(fileHandle);
    PALAT_DATA.alloc(fileSize, "PALAT data");

    FH_PALAT = fileHandle;
    FPOS_PALAT = kfile_position(fileHandle);
    FLEN_PALAT = fileSize;

    kfile_read_byte_array(PALAT_DATA.ptr(), PALAT_DATA.up_to(fileSize), fileHandle);

    // Split the big array of pixel data into individual PALA textures. Note that the
    // textures' pixel pointers will point to this array rather than duplicating it.
    for (uint i = 0; i < MAX_NUM_PALAS; i++)
    {
        texture_c t;
        t.initialize({PALA_WIDTH, PALA_HEIGHT, 8}, "PALA texture (filtered)", false);
        t.isFiltered = true;

        const uint palaSize = PALA_WIDTH * PALA_HEIGHT;
        const uint palaIdx = i * palaSize;

        k_assert(((palaIdx + palaSize) <= fileSize),
                 "Detected an overflow condition in the PALA buffer.");

        // Disable filtering on billboards - I think it looks better that way.
        if ((i >= 208 && i <= 212) ||
            (i >= 236 && i <= 239) ||
            i == 219)
        {
            t.isFiltered = false;
        }

        // Disable filtering for bridge surfaces.
        if (i == 177)
        {
            t.isFiltered = false;
        }

        const uint offs = i * (PALA_WIDTH * PALA_HEIGHT); // Offset in the PALAT pixel array where this particular PALA starts.
        t.pixels.point_to(&PALAT_DATA[offs], (PALAT_DATA.size() - offs));

        // The Matrox Mystique always has filtering disabled, so we don't need to
        // upload filtered textures for it. For other renderers, go ahead and do so.
#ifndef RENDERER_IS_MSI
        t.make_available_to_renderer();
        PALAT.push_back(t);
#endif

        // For convenience, also create an unfiltered version of the texture.
        /// TODO. We shouldn't really need to create a whole new texture for this, since
        /// it's just toggling the filtering flag.
        texture_c tUnfilt;
        tUnfilt.initialize({PALA_WIDTH, PALA_HEIGHT, 8}, "PALA texture (unfiltered)", false);
        tUnfilt.pixels.point_to(&PALAT_DATA[offs], (PALAT_DATA.size() - offs));
        tUnfilt.isFiltered = false;
        tUnfilt.make_available_to_renderer();
        PALAT_UNFILTERED.push_back(tUnfilt);
    }

    NULL_PALA.initialize({PALA_WIDTH, PALA_HEIGHT, 8}, "Null PALA texture");

    return;
}

void kpalat_release_palat(void)
{
#ifndef VALIDATION_RUN
    k_assert(kmain_program_is_exiting(),
             "Was asked to release PALA data before the program had been told to exit.");
#endif

    PALAT_DATA.release_memory();
    NULL_PALA.pixels.release_memory();

    return;
}

uint kpalat_max_num_palas(void)
{
    return MAX_NUM_PALAS;
}

// Returns the number of PALA textures we've got loaded.
//
uint kpalat_num_palas(void)
{
    return PALAT.size();
}

uint kpalat_pala_width(void)
{
    return PALA_WIDTH;
}

uint kpalat_pala_height(void)
{
    return PALA_HEIGHT;
}
