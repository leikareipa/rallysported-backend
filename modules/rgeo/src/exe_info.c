/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Provides information (like byte offsets) related to Rally-Sport's executables.
 * 
 * To understand the Rally-Sport-related terminology used here, you are expected
 * to have read - or to reference as needed - the documentation on Rally-Sport's
 * data formats, available at github.com/leikareipa/rallysported/blob/master/docs/rs-formats.txt.
 * 
 */

#include "exe_info.h"
#include "common/globals.h"
#include "types.h"

#define TEST_BOUNDS(trackIdx) k_assert(((trackIdx) < 8), "Attempting to access executable information out of bounds.")

/* Starting offset of the given track's header block in RALLYE.EXE.*/
u32 kexe_rallye_track_header_block_start(const u8 trackIdx)
{
    TEST_BOUNDS(trackIdx);
    return 0x15045 + 18 * trackIdx;
}

/* Starting offset of the given track's header block in VALIKKO.EXE.*/
u32 kexe_valikko_track_header_block_start(const u8 trackIdx)
{
    TEST_BOUNDS(trackIdx);
    return 0x1a176 + 18 * trackIdx;
}

/* Byte size of the RALLYE.EXE file.*/
u32 kexe_rallye_executable_file_size(void)
{
    return 133452;
}

/* Returns the index of the PALAT file used by the track of the given index.
 * The track index is expected in the range 1..n, i.e. with 1-based indexing;
 * and the value returned for the PALAT file will likewise be a 1-based index.*/
uint kexe_pala_index_for_track_index(const uint trackIdx)
{
    k_assert((trackIdx > 0 && trackIdx <= KEXE_NUM_TRACKS),
             "Tried to query the PALAT index with a track index out of bounds.");

    switch (trackIdx)
    {
        case 5: return 2;
        default: return 1;
    }
}

/* Starting offset of the given track's prop block in RALLYE.EXE.*/
u32 kexe_rallye_prop_block_start(const u8 trackIdx)
{
    TEST_BOUNDS(trackIdx);
    switch (trackIdx)
    {
        case 0: return 0x15109;
        case 1: return 0x1518f;
        case 2: return 0x1519d;
        case 3: return 0x151cf;
        case 4: return 0x15279;
        case 5: return 0x15293;
        case 6: return 0x152a1;
        case 7: return 0x1533f;
        default: k_assert(0, "Unhandled case."); return 0;
    }
}

/* Offset of one byte past the end of a given track's prop block in RALLYE.EXE.*/
u32 kexe_rallye_prop_block_end(const u8 trackIdx)
{
    TEST_BOUNDS(trackIdx);
    return (trackIdx < 7? kexe_rallye_prop_block_start(trackIdx+1) : 0x153d1);
}

/* Starting offset of the given track's car starting position block in RALLYE.EXE.*/
u32 kexe_rallye_starting_position_block(const u8 trackIdx)
{
    TEST_BOUNDS(trackIdx);
    switch (trackIdx)
    {
        case 0: return 0x1500d;
        case 1: return 0x1500d;
        case 2: return 0x1500d;
        case 3: return 0x15015;
        case 4: return 0x1501d;
        case 5: return 0x15025;
        case 6: return 0x1502d;
        case 7: return 0x15035;
        default: k_assert(0, "Unhandled case."); return 0;
    }
}

/* Number of bytes in a prop block.*/
u32 kexe_rallye_prop_block_length(void)
{
    return 12;
}

/* Number of props on the given track, by default.*/
u32 kexe_rallye_default_prop_count(const u8 trackIdx)
{
    switch (trackIdx)
    {
        case 0: return 11;
        case 1: return 1;
        case 2: return 4;
        case 3: return 14;
        case 4: return 2;
        case 5: return 1;
        case 6: return 13;
        case 7: return 12;
        default: k_assert(0, "Unhandled case."); return 0;
    }
}

/* Starting offset of the given prop on the given track.*/
u32 kexe_rallye_prop_offset(const u8 propIdx, const u8 trackIdx)
{
    u32 offs = 0;

    TEST_BOUNDS(trackIdx);
    k_assert((propIdx < kexe_rallye_default_prop_count(trackIdx)), "Attempting to access prop data out of bounds.");

    switch (trackIdx)
    {
        case 0: offs = 0x1510b;
        case 1: offs = 0x15191;
        case 2: offs = 0x1519f;
        case 3: offs = 0x151d1;
        case 4: offs = 0x1527b;
        case 5: offs = 0x15295;
        case 6: offs = 0x152a3;
        case 7: offs = 0x15341;
        default: k_assert(0, "Unhandled case."); return 0;
    }

    return (offs + kexe_rallye_prop_block_length() * (propIdx - 1));
}

/* The given prop type's id string. The id string is a unique type identifier,
 * which can be used to distinguish between the different prop types.*/
const u8* kexe_rallye_prop_type_id(const u8 propType)
{
    static const u8 propIds[16][6] = {{0x2c, 0xd0, 0x88, 0xd0, 0x46, 0x42},  /* Tree.*/
                                      {0xe2, 0x47, 0x98, 0x4c, 0x26, 0x42},  /* Fence (chicken wire).*/
                                      {0xe2, 0x47, 0x98, 0x4d, 0x26, 0x42},  /* Fence (horse).*/
                                      {0x66, 0x47, 0x20, 0x4c, 0xde, 0x42},  /* Traffic sign: '80'.*/
                                      {0x66, 0x47, 0x5c, 0x4c, 0xde, 0x42},  /* Traffic sign: '!'.*/
                                      {0xf2, 0x4f, 0xe0, 0x51, 0x84, 0x42},  /* Stone arch.*/
                                      {0xba, 0x4a, 0xec, 0x4a, 0xfe, 0x42},  /* Stone post.*/
                                      {0x32, 0x49, 0xe4, 0x49, 0x0e, 0x43},  /* Rock: large.*/
                                      {0x8e, 0x49, 0xe4, 0x49, 0x0e, 0x43},  /* Rock: small.*/
                                      {0x66, 0x44, 0x60, 0x45, 0xa2, 0x42},  /* Billboard: large.*/
                                      {0xe6, 0x44, 0x60, 0x46, 0xc0, 0x42},  /* Billboard: small.*/
                                      {0xee, 0x48, 0x7c, 0x4b, 0x16, 0x42},  /* House.*/
                                      {0x24, 0x43, 0x4a, 0x43, 0xee, 0x42},  /* Utility pole 1 (not incl. wiring).*/
                                      {0x24, 0x43, 0x9c, 0x43, 0xee, 0x42},  /* Utility pole 2 (not incl. wiring).*/
                                      {0x88, 0x54, 0x02, 0x55, 0x66, 0x42},  /* Finish: normal.*/
                                      {0xd2, 0x50, 0xc4, 0x51, 0x84, 0x42}}; /* Finish: stone arch.*/

    k_assert((propType < 16), "Attempting to access prop ids out of bounds.");

    return propIds[propType];
}
