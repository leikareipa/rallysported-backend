/*
 * 2017, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED game exe info
 *
 * Provides named offsets to hard-coded data in Rally-Sport's executables.
 *
 */

#include <cstring>
#include <cstdio>
#include "../../core/game_exe.h"
#include "../../core/display.h"
#include "../../core/common.h"
#include "../../core/file.h"
#include "../../core/main.h"

static const char SANDBOXED_RALLYE_EXE_NAME[] = "~~RGEO_RALLYE.EXE";

// The file sizes (bytes) we expect the game's executables to be. These are
// for the game's demo version.
static const u32 F_EXPECTED_SIZE_RALLYE = 133452;
static const u32 F_EXPECTED_SIZE_VALIKKO = 153871;

// The actual file sizes (bytes) that we find when we open the files.
static u32 F_SIZE_RALLYE = 0;
static u32 F_SIZE_VALIKKO = 0;

// File handles.
static file_handle_t FH_RALLYE_EXE = 0;
static file_handle_t FH_VALIKKO_EXE = 0;

file_handle_t kge_file_handle_rallye_exe(void)
{
    k_assert(F_SIZE_RALLYE != 0,
             "Was asked to provide the file handle for RALLYE.EXE before it had been initialized. Can't do that.");

    return FH_RALLYE_EXE;
}

file_handle_t kge_file_handle_valikko_exe(void)
{
    k_assert(F_SIZE_VALIKKO != 0,
             "Was asked to provide the file handle for VALIKKO.EXE before it had been initialized. Can't do that.");

    return FH_VALIKKO_EXE;
}

// Returns the actual offset in the given executable of the given type of offset.
// Optional parameters can be provided.
//
u32 kge_game_exe_offset(const exe_id_e exe, const exe_offset_e offsType, const u8 param, const u8 param2)
{
    u32 offs = 0;

    switch (offsType)
    {
        case GE_OFFS_PROP_TEXTURE_UVS:
        {
            if (exe == GE_RALLYE_EXE)
            {
                offs = 123614;
            }
            else
            {
                k_assert(0, "Cannot provide the prop offset for VALIKKO.EXE");
            }

            break;
        }

        case GE_OFFS_TRACK_HEADER_BLOCK:
        {
            if (exe == GE_RALLYE_EXE)
            {
                offs = 0x15045 + (18 * (param - 1));    // Offset of first track's header + skipover.
            }
            else
            {
                k_assert(0, "Cannot provide the prop offset for VALIKKO.EXE");
            }

            break;
        }

        case GE_OFFS_TRACK_WATER_LEVEL:
        {
            offs = kge_game_exe_offset(exe, GE_OFFS_TRACK_HEADER_BLOCK, param);

            // Seek to the water level byte.
            offs += 8;

            break;
        }

        case GE_OFFS_TRACK_PROP_BLOCK:
        {
            if (exe == GE_RALLYE_EXE)
            {
                switch (param)  // Map id.
                {
                    case 1: { offs = 0x15109; break; }
                    case 2: { offs = 0x1518f; break; }
                    case 3: { offs = 0x1519d; break; }
                    case 4: { offs = 0x151cf; break; }
                    case 5: { offs = 0x15279; break; }
                    case 6: { offs = 0x15293; break; }
                    case 7: { offs = 0x152a1; break; }
                    case 8: { offs = 0x1533f; break; }

                    default: { k_assert(0, "Unknown map id."); }
                }
            }
            else
            {
                k_assert(0, "Cannot provide the prop offset for VALIKKO.EXE");
            }

            break;
        }

        case GE_OFFS_TRACK_PROP_BLOCK_END:
        {
            if (exe == GE_RALLYE_EXE)
            {
                switch (param)  // Map id.
                {
                    case 1: { offs = 0x1518f; break; }
                    case 2: { offs = 0x1519d; break; }
                    case 3: { offs = 0x151cf; break; }
                    case 4: { offs = 0x15279; break; }
                    case 5: { offs = 0x15293; break; }
                    case 6: { offs = 0x152a1; break; }
                    case 7: { offs = 0x1533f; break; }
                    case 8: { offs = 0x153d1; break; }

                    default: { k_assert(0, "Unknown map id."); }
                }
            }
            else
            {
                k_assert(0, "Cannot provide the prop offset for VALIKKO.EXE");
            }

            break;
        }

        case GE_OFFS_STARTING_POSITION:
        {
            if (exe == GE_RALLYE_EXE)
            {
                switch (param)  // Map id.
                {
                    case 1: { offs = 0x1500d; break; }
                    case 2: { offs = 0x1500d; break; }
                    case 3: { offs = 0x1500d; break; }
                    case 4: { offs = 0x15015; break; }
                    case 5: { offs = 0x1501d; break; }
                    case 6: { offs = 0x15025; break; }
                    case 7: { offs = 0x1502d; break; }
                    case 8: { offs = 0x15035; break; }

                    default: { k_assert(0, "Unknown map id for fetching hard-coded starting position data. Can't proceed."); }
                }
            }
            else
            {
                k_assert(0, "Cannot provide the prop offset for VALIKKO.EXE");
            }

            break;
        }

        case GE_OFFS_TRACK_PROP:
        {
            if (exe == GE_RALLYE_EXE)
            {
                switch (param)  // Map id.
                {
                    case 1: { offs = 0x1510b; break; }
                    case 2: { offs = 0x15191; break; }
                    case 3: { offs = 0x1519f; break; }
                    case 4: { offs = 0x151d1; break; }
                    case 5: { offs = 0x1527b; break; }
                    case 6: { offs = 0x15295; break; }
                    case 7: { offs = 0x152a3; break; }
                    case 8: { offs = 0x15341; break; }

                    default: { k_assert(0, "Unknown map id for fetching hard-coded track data. Can't proceed."); }
                }

                offs += RALLYE_NUM_BYTES_IN_PROP_BLOCK * (param2 - 1);
            }
            else
            {
                k_assert(0, "Cannot provide the prop offset for VALIKKO.EXE");
            }

            break;
        }

        case GE_OFFS_DATA_SEGMENT:
        {
            if (exe == GE_RALLYE_EXE)
            {
                offs = 70560;
            }
            else
            {
                k_assert(0, "Cannot provide the data block offset for VALIKKO.EXE");
            }

            break;
        }

        case GE_OFFS_PALETTE:
        {
            if (exe == GE_RALLYE_EXE)
            {
                offs = 131798;
            }
            else
            {
                k_assert(0, "Cannot provide the palette offset for VALIKKO.EXE");
            }

            break;
        }

        case GE_OFFS_STRING_LOCATION_MAASTO:
        {
            if (exe == GE_RALLYE_EXE)
            {
                offs = 0x2086f;
            }
            else
            {
                k_assert(0, "Cannot provide the palette offset for VALIKKO.EXE");
            }

            break;
        }

        case GE_OFFS_STRING_LOCATION_VARIMAA:
        {
            if (exe == GE_RALLYE_EXE)
            {
                offs = 0x2087a;
            }
            else
            {
                k_assert(0, "Cannot provide the palette offset for VALIKKO.EXE");
            }

            break;
        }

        case GE_OFFS_STRING_LOCATION_PALAT:
        {
            if (exe == GE_RALLYE_EXE)
            {
                offs = 0x20886;
            }
            else
            {
                k_assert(0, "Cannot provide the palette offset for VALIKKO.EXE");
            }

            break;
        }

        default: break;
    }

    k_assert(offs != 0,
             "Having to return 0 as a game executable offset - this seems fishy.");

    if (exe == GE_RALLYE_EXE)
    {
        k_assert(offs < F_EXPECTED_SIZE_RALLYE,
                 "Was sked to return a game executable offset out of bounds. A calculation is off somewhere.");
    }
    else if (exe == GE_VALIKKO_EXE)
    {
        k_assert(offs < F_EXPECTED_SIZE_VALIKKO,
                 "Was sked to return a game executable offset out of bounds. A calculation is off somewhere.");
    }

    return offs;
}

// Open the game's executables for read access.
//
void kge_acquire_read_access_to_game_executables(const char *const fnRallye,
                                                 const char *const fnValikko)
{
    const file_handle_t fhOrigRallye = kfile_open_file(fnRallye, "rb");
    FH_RALLYE_EXE = kfile_open_file(SANDBOXED_RALLYE_EXE_NAME, "wb+");    // Create a sandboxed version of the game executable.
    FH_VALIKKO_EXE = kfile_open_file(fnValikko, "rb");

    kfile_append_contents(fhOrigRallye, FH_RALLYE_EXE);

    F_SIZE_RALLYE = kfile_file_size(FH_RALLYE_EXE);
    F_SIZE_VALIKKO = kfile_file_size(FH_VALIKKO_EXE);

    k_assert(F_SIZE_RALLYE == F_EXPECTED_SIZE_RALLYE,
             "The file RALLYE.EXE is not of the expected size. Is this not the demo version of Rally-Sport?");

    k_assert(F_SIZE_VALIKKO == F_EXPECTED_SIZE_VALIKKO,
             "The file VALIKKO.EXE is not of the expected size. Is this not the demo version of Rally-Sport?");

    return;
}

void kge_release_game_executables(void)
{
    k_assert(kmain_program_is_exiting(),
             "Was asked to relinquish access to game executables before the program had been told to exit.");

    kfile_close_file(FH_RALLYE_EXE);
    kfile_close_file(FH_VALIKKO_EXE);

    return;
}
