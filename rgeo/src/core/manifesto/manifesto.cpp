/*
 * 2017, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED manifesto file handler
 *
 * Loads, parses, and exports RallySportED's manifesto (.$FT) files.
 *
 * The manifesto file contains a set of commands for manipulating hardcoded
 * parameters in Rally-Sport's executables, for the benefit of obtaining desired
 * functionality for a particular modded track.
 *
 */

#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include "../../core/manifesto.h"
#include "../../core/game_exe.h"
#include "../../core/display.h"
#include "../../core/palette.h"
#include "../../core/common.h"
#include "../../core/file.h"

/*
 * TODOS:
 *
 * - mainly uses C-style strings for legacy reasons. maybe change that some day.
 *
 */

// The id of the track we've loaded in. This will be adjusted according to commands
// in the manifesto file.
static u8 TRACK_IDX = 1;

static file_handle_t FH_MANIFESTO = 0;                         // A handle to the manifesto file. Will be set when we're asked to load a manifesto.

static std::vector<std::string> MANIFESTO_COMMANDS;            // A list of commands and their parameters that we've loaded from the current manifesto file.
static std::vector<std::string> MANIFESTO_COMMANDS_EXTRA;      // Commands not present when the manifesto was loaded from disk, but which will be added on export.

// Creates a manifesto command based on the given input, which will then be saved
// into the manifesto file when the track is exported. Note that any commands added
// through this function will be erased once the track has been exported - you'll
// want to call it again each time before exporting.
//
void kmanif_add_command_to_manifesto(const manifesto_command_e cmd, const u8 *const params)
{
    int r = 0;
    char line[128];

    // Construct the line based on which manifesto command we're dealing with.
    switch (cmd)
    {
        case MANIFCMD_MOVE_PROP:
        {
            r = snprintf(line, NUM_ELEMENTS(line), "5 %u %u %u %u %u", params[0], params[1], params[2], params[3], params[4]);
            break;
        }
        case MANIFCMD_SET_PROP:
        {
            r = snprintf(line, NUM_ELEMENTS(line), "4 %u %u", params[0], params[1]);
            break;
        }
        case MANIFCMD_NUM_PROPS:
        {
            r = snprintf(line, NUM_ELEMENTS(line), "2 %u", params[0]);
            break;
        }
        default:
        {
            k_assert(0, "Was asked to add an unknown manifesto command. Can't do it.");
            break;
        }
    }

    k_assert((r >= 0) && (r < NUM_ELEMENTS(line)), "Failed to add a new manifesto line. The string buffer was too small.");

    MANIFESTO_COMMANDS_EXTRA.push_back(line);

    return;
}

// Overwrites the original manifesto file with what we currently have in store.
//
void kmanif_save_manifesto_file(void)
{
    kfile_rewind_file(FH_MANIFESTO);

    for (uint i = 0; i < MANIFESTO_COMMANDS.size(); i++)
    {
        kfile_write_string(MANIFESTO_COMMANDS[i].c_str(), FH_MANIFESTO);
        kfile_write_string("\n", FH_MANIFESTO);
    }
    for (uint i = 0; i < MANIFESTO_COMMANDS_EXTRA.size(); i++)
    {
        kfile_write_string(MANIFESTO_COMMANDS_EXTRA[i].c_str(), FH_MANIFESTO);
        kfile_write_string("\n", FH_MANIFESTO);
    }
    kfile_write_string("99\n\n", FH_MANIFESTO);
    kfile_flush_file(FH_MANIFESTO);

    MANIFESTO_COMMANDS_EXTRA.clear();  // We want the extra lines to be redefined every time before this function is called again.

    return;
}

// Extracts a set of unsigned manifesto command parameters from the given string.
//
static int extract_params(const char *const command, uint *const params, const uint maxNumParams)
{
    uint numParams = 0;

    /// C-style string parsing for legacy reasons.
    int paramNum = 0;
    while (command[paramNum] != '\0')
    {
        int c = 0;
        char paramValStr[5]; // Biggest possible value = 9999, with null term.

        while (1)
        {
            paramValStr[c] = '\0';

            if (command[paramNum] == '\0')    // End of the command line.
            {
                break;
            }
            else if (command[paramNum] < 48)  // Assume space or newline which marks the start of a new parameter.
            {
                paramNum++;
                break;
            }

            paramValStr[c++] = command[paramNum++];

            if (c >= NUM_ELEMENTS(paramValStr))
            {
                return -1;
            }
        }

        params[numParams] = strtoul(paramValStr, NULL, 0);

        if (numParams >= maxNumParams)
        {
            return -1;
        }

        numParams++;
    }

    return numParams;
}

u8 kmanif_track_idx(void)
{
    return TRACK_IDX;
}

// Modifies certain hard-coded data of the given target file (assumed RALLYE.EXE)
// according to the manifesto's commands. The target file is assumed to be a copy
// of the actual file, so that we don't permanently modify the game but only a
// sandbox of it.
//
bool kmanif_apply_manifesto(const char *const manifestoFilename,
                            const file_handle_t targetFh)
{
    FH_MANIFESTO = kfile_open_file(manifestoFilename, "rb+");

    u8 propsAdded = 0;	// How many custom prop we've added. Used by MANIFCMD_ADD_PROP.

    // Parse through the commands in the manifesto, putting each into effect.
    int currentCommand = MANIFCMD_NONE;
    while (currentCommand != MANIFCMD_STOP)
    {
        static uint lineNum = 0;
        lineNum++;

        char nextCommand[256] = {0};
        if (!kfile_getline(FH_MANIFESTO, nextCommand, NUM_ELEMENTS(nextCommand)))
        {
            printf("ERROR: IO error on manifesto line %u, aborting.\n", lineNum);
            k_assert(0, "Failed parsing the manifesto file. Can't continue.");

            return false;
        }

        uint cmdParams[8];
        const int numParams = (extract_params(nextCommand, cmdParams, NUM_ELEMENTS(cmdParams)) - 1);    // -1, since the first param is the command itself.
        if (numParams == -1)
        {
            printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
            k_assert(0, "Unexpected parameters for MANIFCMD_STOP in the manifesto file. Can't continue.");

            return false;
        }

        // The first parameter is the command itself.
        currentCommand = cmdParams[0];

        // Execute the command.
        if (currentCommand == MANIFCMD_STOP)
        {
            break;
        }
        else if (currentCommand == MANIFCMD_REQUIRE)
        {
            MANIFESTO_COMMANDS.push_back(nextCommand);

            if (numParams != 3)
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_REQUIRE in the manifesto file. Can't continue.");

                return false;
            }

            if (LOADER_MAJOR_VERSION < cmdParams[3])
            {
                printf("ERROR: Track requires RSED Loader v.%i or later, but your RSED Loader is v.%i. "
                       "Get the latest version at http://personal.inet.fi/muoti/eimuoti/rsed/\n",
                       cmdParams[3], LOADER_MAJOR_VERSION);
                k_assert(0, "RSED version mismatch. Can't continue.");

                return false;
            }

            // Expect valid ranges for the Rally-Sport demo.
            if (cmdParams[1] < 1 || cmdParams[1] > 8 ||
                cmdParams[2] < 1 || cmdParams[2] > 2)
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_REQUIRE in the manifesto file. Can't continue.");

                return false;
            }

            TRACK_IDX = cmdParams[1];// - 1;

            u8 maastoExt = cmdParams[1] + '0';
            u8 palatExt = cmdParams[2] + '0';

            // Write the file extensions to RALLYE.EXE, so we can pick them up from there when
            // we do functional patching.
            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_STRING_LOCATION_MAASTO) + 9, targetFh);
            kfile_write_value<u8>(maastoExt, targetFh);
            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_STRING_LOCATION_VARIMAA) + 10, targetFh);
            kfile_write_value<u8>(maastoExt, targetFh);
            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_STRING_LOCATION_PALAT) + 8, targetFh);
            kfile_write_value<u8>(palatExt, targetFh);

            // Also update the track's header block.
            palatExt -= '1'; // Convert to int and 0-index.
            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_TRACK_HEADER_BLOCK, TRACK_IDX) + 10, targetFh);
            kfile_write_value<u8>(palatExt, targetFh);
        }
        else if (currentCommand == MANIFCMD_NUM_PROPS)
        {
            //MANIFESTO_LINES.push_back(nextLine);

            //DEBUG_OUTPUT(("\tCommand: Num_objs\n"));

            if (numParams != 1)
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_NUM_PROPS in the manifesto file. Can't continue.");

                return false;
            }

            // Need at least one prop (starting line). For now, limit to 26 props per map.
            if (cmdParams[1] < 1 || cmdParams[1] > 26)
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_NUM_PROPS in the manifesto file. Can't continue.");

                return false;
            }

            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_TRACK_PROP_BLOCK, TRACK_IDX), targetFh);
            kfile_write_value<u8>(cmdParams[1], targetFh);
        }
        /*else if (cmd == MANIFCMD_MOVE_STARTING_POS) // Note: Use this in conjunction with MOVE_OBJ to move the starting line, as well.
        {
            MANIFESTO_COMMANDS.push_back(nextCommand);

            //DEBUG_OUTPUT(("\tCommand: Move_starting_pos\n"));

            if (numParams != 4)
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_MOVE_STARTING_POS in the manifesto file. Can't continue.");

                return false;
            }

            if (cmdParams[1] < 5 || cmdParams[1] > 58 ||  // X pos. These coordinates are multiplied by two in the game.
                cmdParams[2] < 5 || cmdParams[2] > 58 ||  // Y pos.
                cmdParams[4] > 255 || // Local X pos.
                cmdParams[5] > 255)   // Local Y pos.
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_MOVE_STARTING_POS in the manifesto file. Can't continue.");

                return false;
            }

            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_STARTING_POSITION, MAP_ID) + 1, targetFH);
            kfile_write_byte_array((u8*)&cmdParams[1], 1, targetFH);
            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_STARTING_POSITION, MAP_ID) + 3, targetFH);
            kfile_write_byte_array((u8*)&cmdParams[2], 1, targetFH);
            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_STARTING_POSITION, MAP_ID) + 0, targetFH);
            kfile_write_byte_array((u8*)&cmdParams[3], 1, targetFH);
            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_STARTING_POSITION, MAP_ID) + 2, targetFH);
            kfile_write_byte_array((u8*)&cmdParams[4], 1, targetFH);
        }*/
        else if (currentCommand == MANIFCMD_MOVE_PROP)
        {
            if (numParams != 5)
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_MOVE_PROP in the manifesto file. Can't continue.");

                return false;
            }

            if (cmdParams[1] < 1 || cmdParams[1] > 20 ||    // Prop type (as an index to the game's hard-coded list of props).
                /*cmdParams[2] > 63 ||*/                    // Global X pos.
                /*cmdParams[3] > 63 ||*/                    // Global Y pos.
                cmdParams[4] > 255 ||                       // Local X pos.
                cmdParams[5] > 255)                         // Local Y pos.
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_MOVE_PROP in the manifesto file. Can't continue.");

                return false;
            }

            const long offs = kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_TRACK_PROP, TRACK_IDX, cmdParams[1]);//ExeInfo_RALLYE::propOffset(params[1], MAP_ID);

            kfile_seek(offs + 7, targetFh);
            kfile_write_value<u8>(cmdParams[2], targetFh);
            kfile_seek(offs + 9, targetFh);
            kfile_write_value<u8>(cmdParams[3], targetFh);
            kfile_seek(offs + 6, targetFh);
            kfile_write_value<u8>(cmdParams[4], targetFh);
            kfile_seek(offs + 8, targetFh);
            kfile_write_value<u8>(cmdParams[5], targetFh);
        }
        else if (currentCommand == MANIFCMD_ADD_PROP)
        {
            MANIFESTO_COMMANDS.push_back(nextCommand);

            if (numParams != 5)
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_ADD_PROP in the manifesto file. Can't continue.");

                return false;
            }

            if (cmdParams[1] < 1 || cmdParams[1] > 20 ||    // Prop type (as an index to the game's hard-coded list of props).
                cmdParams[2] > 64 ||                        // Global x.
                cmdParams[3] > 64 ||                        // Global y.
                cmdParams[4] > 255 ||                       // Local x.
                cmdParams[5] > 255)                         // Local y.
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_ADD_PROP in the manifesto file. Can't continue.");

                return false;
            }

            u8 newObj[RALLYE_NUM_BYTES_IN_PROP_BLOCK];
            kge_prop_id_string(newObj, (u8)cmdParams[1]);

            newObj[6] = cmdParams[4];
            newObj[7] = cmdParams[2];
            newObj[8] = cmdParams[5];
            newObj[9] = cmdParams[3];
            newObj[10] = 0xff;      /// Height, defaults to 255 for now.
            newObj[11] = 0xff;

            // Increase the count of props in RALLYE.EXE by one.
            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_TRACK_PROP_BLOCK, TRACK_IDX), targetFh);
            u8 numObjs = kfile_read_value<u8>(targetFh);
            numObjs++;

            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_TRACK_PROP_BLOCK, TRACK_IDX), targetFh);
            kfile_write_value<u8>(numObjs, targetFh);
            kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_TRACK_PROP_BLOCK_END, TRACK_IDX) + propsAdded * 12, targetFh);
            kfile_write_byte_array(newObj, sizeof(newObj), targetFh);   // Overwrite some of the next track's data. Should be ok since we run the game with just one track at a time. As long as this isn't the last track.

            propsAdded++;
        }
        else if (currentCommand == MANIFCMD_SET_PROP)
        {
            if (numParams != 2)
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_SET_PROP in the manifesto file. Can't continue.");

                return false;
            }

            // Need at least one prop (starting line). For now, limit to 12 props per map.
            if (cmdParams[1] < 1 || cmdParams[1] > 14 ||  // prop index. For now, assume a maximum of 14 props per map.
                cmdParams[2] < 1 || cmdParams[2] > 16)    // prop type.
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_SET_PROP in the manifesto file. Can't continue.");

                return false;
            }

            u8 o[RALLYE_NUM_BYTES_IN_PROP_BLOCK];

            long offs = kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_TRACK_PROP, TRACK_IDX, (u8)cmdParams[1]);
            if (offs == -1)
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_SET_PROP in the manifesto file. Can't continue.");

                return false;
            }

            // Read in the entire prop header.
            kfile_seek(offs, targetFh);
            kfile_read_byte_array(o, RALLYE_NUM_BYTES_IN_PROP_BLOCK, targetFh);

            // Change the type-relevant bytes in the header to get the desired prop type.
            kge_prop_id_string(o, cmdParams[2]);
            kfile_seek(offs, targetFh);
            kfile_write_byte_array(o, RALLYE_NUM_BYTES_IN_PROP_BLOCK, targetFh);
        }
        else if (currentCommand == MANIFCMD_SET_COLOR)
        {
            MANIFESTO_COMMANDS.push_back(nextCommand);

            if (numParams != 4)
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_SET_COLOR in the manifesto file. Can't continue.");

                return false;
            }

            if (cmdParams[1] > 31 ||        // Palette index.
                cmdParams[2] > 63 ||        // New red.
                cmdParams[3] > 63 ||        // New green.
                cmdParams[4] > 63)          // New blue.
            {
                printf("ERROR: Unexpected manifesto parameters on line %u, aborting.\n", lineNum);
                k_assert(0, "Unexpected parameters for MANIFCMD_SET_COLOR in the manifesto file. Can't continue.");

                return false;
            }

            // Establish which palette the track is using.
            u8 paletteIdx = 0;
            if (TRACK_IDX == 5) paletteIdx = 1;
            else if (TRACK_IDX == 8) paletteIdx = 3;

            // Seek from the start of the palette block to the current color.
            /// TODO. Add a thing to kge_game_exe_offset() that returns a palette color offset directly.
            const u32 offs = kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_PALETTE) +
                             (paletteIdx * kpal_num_palettes() * kpal_num_primary_colors()) + (3 * cmdParams[1]);

            kfile_seek(offs, targetFh);
            kfile_write_value<u8>(cmdParams[2], targetFh);
            kfile_seek(offs + 1, targetFh);
            kfile_write_value<u8>(cmdParams[3], targetFh);
            kfile_seek(offs + 2, targetFh);
            kfile_write_value<u8>(cmdParams[4], targetFh);
        }
        else
        {
            k_assert(0, "Detected an unknown command in the manifesto file. Can't continue.");
        }
    }

    return true;
}
