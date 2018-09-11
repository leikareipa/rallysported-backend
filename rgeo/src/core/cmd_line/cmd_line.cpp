/*
 * 2016-2018 Tarpeeksi Hyvae Soft /
 * RallySportED command line
 *
 * Parses command line parameters into actions.
 *
 */

#include <cstring>
#include <cstdlib>
#include <cctype>
#include "../../core/cmd_line.h"
#include "../../core/display.h"
#include "../../core/maasto.h"
#include "../../core/memory.h"
#include "../../core/common.h"
#include "../../core/types.h"
#include "../../core/file.h"

/*
 * TODOS:
 *
 * - there's a bunch of functionality here, like the creation of a new track, that
 *   really should be in some other, more suitable unit.
 *
 */

// The id # of the track we've been asked to open to create. This value corresponds
// to the track numbering in Rally-Sport such that e.g. 1 refers to 'nurtsi cruising'.
static uint TRACK_ID = 0;

static const uint MAX_TRACK_NAME_LEN = 8;
static char TRACK_NAME[MAX_TRACK_NAME_LEN + 1] = {0};

const char* kcmdl_track_name(void)
{
    return TRACK_NAME;
}

/// Todo. This needs to be in its own unit, not in the command line one.
static void create_new_project(const char *const projName)
{
    char strBuf[256];
    file_handle_t tmpFh;
    const char trackId = TRACK_ID + '0';                // MAASTO file extension (the significant number).
    const char palatId = (trackId == '5'? '2' : '1');   // PALAT file extension (the significant number).

    // Create the container file to hold the map's assets.
    kfile_create_directory(projName, false);
    snprintf(strBuf, NUM_ELEMENTS(strBuf), "%s%c%s.DTA", projName, DIR_SEPARATOR, projName);
    const file_handle_t fhProj = kfile_open_file(strBuf, "wb");

    // Insert the given track's files into the container.
    auto append_to_project = [fhProj](const char *const expr, const char var)
                             {
                                 char strBuf[256];
                                 snprintf(strBuf, NUM_ELEMENTS(strBuf), expr, var);
                                 const file_handle_t fh = kfile_open_file(strBuf, "rb");
                                 kfile_write_value<u32>(kfile_file_size(fh), fhProj);
                                 kfile_append_contents(fh, fhProj);
                                 kfile_close_file(fh);
                             };
    append_to_project("MAASTO.00%c", trackId);
    append_to_project("VARIMAA.00%c", trackId);
    append_to_project("PALAT.00%c", palatId);
    append_to_project("ANIMS.DTA", 0);
    append_to_project("TEXT1.DTA", 0);
    append_to_project("KIERROS%c.DTA", trackId);

    // Create a default manifesto file.
    snprintf(strBuf, NUM_ELEMENTS(strBuf), "%s%c%s.$FT", projName, DIR_SEPARATOR, projName);
    tmpFh = kfile_open_file(strBuf, "wb");
    snprintf(strBuf, NUM_ELEMENTS(strBuf), "0 %c %c %c\n99\n\n", trackId, palatId, REQUIRED_LOADER_VER);    // Initialize the manifesto's content.
    kfile_write_string(strBuf, tmpFh);
    kfile_close_file(tmpFh);

    // Create a default HITABLE.TXT file.
    #include "../../core/lut/hitable.c"
    snprintf(strBuf, NUM_ELEMENTS(strBuf), "%s%cHITABLE.TXT", projName, DIR_SEPARATOR);
    tmpFh = kfile_open_file(strBuf, "wb");
    kfile_write_byte_array(HITABLE_TXT, sizeof(HITABLE_TXT), tmpFh);
    kfile_close_file(tmpFh);

    INFO(("Finished creating the new track."));
    INFO(("Bye."));
    exit(EXIT_SUCCESS);

    return;
}

/// Todo. This needs to be in its own unit, not in the command line one.
static void reset_project_heightmap(const char *const projName, const int height)
{
    char strBuf[128];
    file_handle_t fhProj = 0;

    // Load the map's container.
    snprintf(strBuf, NUM_ELEMENTS(strBuf), "%s%c%s.DTA", projName, DIR_SEPARATOR, projName);
    fhProj = kfile_open_file(strBuf, "rb+");

    // Get the size of the MAASTO file (it's the container's first file).
    const u32 maastoSize = kfile_read_value<u32>(fhProj);

    heap_bytes_s<u8> maastoData(maastoSize, "MAASTO data for export");
    kfile_read_byte_array(maastoData.ptr(), maastoData.up_to(maastoSize), fhProj);

    std::pair<u8, u8> hb = kmaasto_editor_height_to_game_height(height);
    for (uint i = 0; i < maastoSize; i += 2)
    {
        maastoData[i] = hb.first;
        maastoData[i + 1] = hb.second;
    }

    kfile_jump(-maastoSize, fhProj);
    kfile_write_byte_array(maastoData.ptr(), maastoData.up_to(maastoSize), fhProj);

    maastoData.release_memory();

    INFO(("Finished resetting the track's heightmap."));
    INFO(("Bye."));
    exit(EXIT_SUCCESS); // We can exit the entire program now.
}

bool kcmdl_parse_command_line(const int argc, const char *const argv[])
{
    int newHeight = 0;
    uint trackNameParam = 0;
    bool createNewProject = false;
    bool resetProjectHeight = false;

    if (argc <= 1)
    {
        printf("ERROR: Expected command line arguments but found none.\n");
        kd_show_headless_info_message("Use the command line to specify the name of the track to be loaded or created.");

        return false;
    }

    if (argc == 2)                  // Expect the name of the track to be opened as the only command line argument.
    {
        trackNameParam = 1;
    }
    else if (argc == 3)             // Expect this to be a request to reset a track's heightmap.
    {
        trackNameParam = 2;

        TRACK_ID = strtol(argv[3], NULL, 10);

        if (argv[1][0] != '-')
        {
            printf("ERROR: Unrecognized command line arguments. Expected one of these:\n\trgeo.exe <track_name>\n\trgeo.exe + <track_name> <track_#>\n"
                   "\trgeo.exe - <track_name>\n");
            kd_show_headless_info_message("Unrecognized command line arguments.");

            return false;
        }

        resetProjectHeight = true;
    }
    else if (argc == 4)             // Expect this to be a request to create a new track, or to reset the height of an existing one.
    {
        trackNameParam = 2;

        if (argv[1][0] == '-')
        {
            newHeight = strtol(argv[3], NULL, 10);

            if (newHeight < -255 ||
                newHeight > 255)
            {
                printf("ERROR: The height given is out of bounds. Expected range: -255..255.\n");
                kd_show_headless_info_message("Was asked to reset a track's heightmap, but the new height that was asked for is out of bounds. Can't do it.");

                return false;
            }

            resetProjectHeight = true;
        }
        else if (argv[1][0] == '+')
        {
            TRACK_ID = strtol(argv[3], NULL, 10);

            if (TRACK_ID > NUM_GAME_TRACKS ||
                TRACK_ID < 1)
            {
                printf("ERROR: Track # is out of bounds. Expected range: 1-%u.\n", NUM_GAME_TRACKS);
                kd_show_headless_info_message("Was asked to create a new track, but the given track id is out of bounds. Can't do it.");

                return false;
            }

            createNewProject = true;
        }
        else
        {
            printf("ERROR: Unrecognized command line arguments. Expected one of these:\n\trgeo.exe <track_name>\n\trgeo.exe + <track_name> <track_#>\n"
                   "\trgeo.exe - <track_name>\n");
            kd_show_headless_info_message("Unrecognized command line arguments.");

            return false;
        }
    }
    else
    {
        printf("ERROR: Unrecognized command line arguments. Expected one of these:\n\trgeo.exe <track_name>\n\trgeo.exe + <track_name> <track_#>\n"
               "\trgeo.exe - <track_name>\n");
        kd_show_headless_info_message("Unrecognized command line arguments.");

        return false;
    }

    // Copy the track name over.
    if (strlen(argv[trackNameParam]) >= NUM_ELEMENTS(TRACK_NAME))
    {
        printf("ERROR: The track name was too long. Maximum length allowed: %d.\n", MAX_TRACK_NAME_LEN);
        kd_show_headless_info_message("The given track name is too long.\n\nRally-Sport is a DOS game,"
                                      " so track names must conform to its limitation of a maximum of eight letters.");

        return false;
    }
    snprintf(TRACK_NAME, NUM_ELEMENTS(TRACK_NAME), "%s", argv[trackNameParam]);
    for (uint i = 0; i < strlen(TRACK_NAME); i++)
    {
        TRACK_NAME[i] = toupper(TRACK_NAME[i]);

        if (TRACK_NAME[i] < 'A' ||
            TRACK_NAME[i] > 'Z')
        {
            printf("ERROR: The track name must contain only the letters A-Z.\n");
            kd_show_headless_info_message("Track names must contain only the letters A-Z.");

            return false;
        }
    }

    // Act out the command-line command.
    if (createNewProject)
    {
        INFO(("Have been asked to create a new track called '%s' (inherits from base track #%u).", TRACK_NAME, TRACK_ID));
        create_new_project(TRACK_NAME);
    }
    else if (resetProjectHeight)
    {
        INFO(("Have been asked to reset the heightmap of track '%s'.", TRACK_NAME));
        reset_project_heightmap(TRACK_NAME, newHeight);
    }

    return true;
}
