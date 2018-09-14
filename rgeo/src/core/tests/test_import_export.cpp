/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED import/export validation
 *
 * Runs a validation test of RGEO's importing and exporting of project data.
 *
 * Note: Does not test the manifesto file, at the moment, only aspects to do
 * with the project's .DTA container.
 *
 */

#ifndef VALIDATION_RUN
    #error "Define VALIDATION_RUN for validation builds."
#endif

#include <algorithm>
#include <sstream>
#include <map>
#include <cstdlib>
#include <cstring>
#include "../../core/manifesto.h"
#include "../../core/game_exe.h"
#include "../../core/display.h"
#include "../../core/texture.h"
#include "../../core/common.h"
#include "../../core/maasto.h"
#include "../../core/palat.h"
#include "../../core/props.h"
#include "../../core/main.h"
#include "../../core/file.h"
#include "../../core/tests/reference_container.h"

/*
 * TODOS:
 *
 * - test the manifesto too.
 *
 */

static const char ASPECT_TO_TEST[] = "Import and export of project data (excluding the manifesto file)";

// The directory into which to create the test project.
static const char TEST_DIR[] = ".~-^``";    /// TODO. Make a better system for this, not a goofily-named hardcoded thing.

// Loops through the data given data, comparing each byte and returning true if
// all bytes are identical; otherwise returns false. You can use the 'reason'
// string to provide a user-facing message for e.g. why this validation is done.
// 
template <typename T>
static void validate_identical(const T *const d1, const T *const d2, const uint len, const char *const reason)
{
    for (uint i = 0; i < len; i++)
    {
        if (d1[i] != d2[i])
        {
            ERRORI(("FAILURE TO VALIDATE: Mismatch at relative offset %u (%u != %u) for '%s'.", i, d1[i], d2[i], reason));
            goto failure;
        }
    }

    success:
    return;

    failure:
    abort();    // Noisily bail out of the program if validation fails.
}

// Returns a string containing the project's name, appended with the local directory
// structure and given file extension (don't include the '.' when passing the latter).
// E.g. "DTA" returns "TRACK/TRACK.DTA".
//
static std::string project_file_name(std::string extension)
{
    std::ostringstream ss;
    ss << TEST_DIR << DIR_SEPARATOR << TEST_DIR << "." << extension;

    return ss.str();
}

static void initialize_rgeo(const reference_container_c &refProject)
{
    DEBUG(("Initializing RGEO..."));

    kge_acquire_access_to_game_executables("RALLYE.EXE", "VALIKKO.EXE");
    kprop_initialize_props();
    kmanif_apply_manifesto(project_file_name("$FT").c_str(), kge_file_handle_rallye_exe());

    kfile_rewind_file(refProject.fileHandle);
    kmaasto_initialize_maasto(refProject.fileHandle, refProject.trackId);
    kpalat_initialize_palat(refProject.fileHandle);

    return;
}

static void release_rgeo(void)
{
    kmain_request_program_exit(EXIT_SUCCESS);
    kmaasto_release_maasto();
    kpalat_release_palat();

    /// TODO. Delete the test project's files and directory from disk.

    return;
}

static void validate_import(const reference_container_c &refProject)
{
    INFO(("Validating import..."));

    const uint impMaastoWidth = kmaasto_track_width();
    const uint impMaastoHeight = kmaasto_track_height();

    // Make sure the game initialized the track's dimensions correctly.
    validate_identical(&impMaastoWidth, &impMaastoHeight, 1, "Track should be square");
    validate_identical(&impMaastoWidth, &refProject.trackSideLen, 1, "Track side length");

    // Validate the MAASTO data.
    for (uint i = 0; i < refProject.element_size("MAASTO")/2; i++)
    {
        const uint y = i / refProject.trackSideLen;
        const uint x = i - (y * refProject.trackSideLen);

        const uint impHeight = kmaasto_height_at(x, y);
        const uint refHeight = kmaasto_game_height_to_editor_height(refProject("MAASTO", i*2),
                                                                    refProject("MAASTO", i*2+1));

        validate_identical(&impHeight, &refHeight, 1, "MAASTO: Height of a track tile");
    }

    // Validate the VARIMAA data.
    for (uint i = 0; i < refProject.element_size("VARIMAA"); i++)
    {
        const uint y = i / refProject.trackSideLen;
        const uint x = i - (y * refProject.trackSideLen);

        const u32 impTexIdx = kmaasto_texture_at(x, y);
        const u32 refTexIdx = refProject("VARIMAA", i);

        validate_identical(&impTexIdx, &refTexIdx, 1, "VARIMAA: Texture index of a track tile");
    }

    // Validate the PALAT data.
    uint pala = 0;
    while (pala < kpalat_max_num_palas())
    {
        const uint palaPixelSize = kpalat_pala_width() * kpalat_pala_height();
        const u8 *impPalaData = kpalat_pala_ptr(pala, false)->pixels.ptr();

        validate_identical(impPalaData, refProject("PALAT"), palaPixelSize, "PALAT: Track textures");

        pala++;
    }

    /// TODO. Validate manifesto data.

    kfile_rewind_file(refProject.fileHandle);

    return;
}

static void validate_export(const reference_container_c &refProject)
{
    INFO(("Validating export..."));

    // Allocate a scratch buffer to load exported data for validation.
    u8 *expData = new u8[refProject.content_size()];

    kfile_rewind_file(refProject.fileHandle);

    const auto announce_elem_offset = [](const std::string &element)
                                      {
                                          DEBUG(("Validating %s (container offset 0x%x)...",
                                                 element.c_str(), int(refProject(element) - refProject("ALL")) - refProject.elementHeaderSize));
                                      };

    // Validate MAASTO data.
    announce_elem_offset("MAASTO");
    const uint expMaastoSize = kfile_read_value<u32>(refProject.fileHandle);
    const uint refMaastoSize = refProject.element_size("MAASTO");
    validate_identical(&expMaastoSize, &refMaastoSize, 1, "MAASTO size");
    kfile_read_byte_array(expData, expMaastoSize, refProject.fileHandle);
    for (uint i = 0; i < expMaastoSize; i += 2)	// Validate in word chunks, for conveniently testing for the special case of b1 == b2 == 255.
    {
        const int expHeight[2] = {expData[i], expData[i+1]};
        int refHeight[2] = {refProject("MAASTO", i), refProject("MAASTO", i+1)};

        // The case b1 == b2 == 255 results in a height of 0 in the game and which will
        // be exported validly as b1 == b2 == 0 by RGEO, so the latter should be the
        // reference here. (It's still useful to test for b1 == b2 == 255 for import,
        // hence why it's generated into the reference data.)
        if (refHeight[0] == 255 && refHeight[1] == 255)
        {
            refHeight[0] = refHeight[1] = 0;
        }

        validate_identical(expHeight, refHeight, 2, "MAASTO: Height of a track tile");
    }

    // Validate VARIMAA data.
    announce_elem_offset("VARIMAA");
    const uint expVarimaaSize = kfile_read_value<u32>(refProject.fileHandle);
    const uint refVarimaaSize = refProject.element_size("VARIMAA");
    validate_identical(&expVarimaaSize, &refVarimaaSize, 1, "VARIMAA size");
    kfile_read_byte_array(expData, expVarimaaSize, refProject.fileHandle);
    validate_identical(expData, refProject("VARIMAA"), expVarimaaSize, "VARIMAA data");

    // Validate PALAT data.
    announce_elem_offset("PALAT");
    const uint expPalatSize = kfile_read_value<u32>(refProject.fileHandle);
    const uint refPalatSize = refProject.element_size("PALAT");
    validate_identical(&expPalatSize, &refPalatSize, 1, "VARIMAA size");
    kfile_read_byte_array(expData, expPalatSize, refProject.fileHandle);
    validate_identical(expData, refProject("PALAT"), expPalatSize, "PALAT data");

    // Validate the rest of the container. RGEO isn't supposed to touch these parts at all.
    announce_elem_offset("ANIMS");
    kfile_read_byte_array(expData, kfile_read_value<u32>(refProject.fileHandle), refProject.fileHandle);
    validate_identical(expData, refProject("ANIMS"), refProject.element_size("ANIMS"), "ANIMS data");

    announce_elem_offset("TEXT1");
    kfile_read_byte_array(expData, kfile_read_value<u32>(refProject.fileHandle), refProject.fileHandle);
    validate_identical(expData, refProject("TEXT1"), refProject.element_size("TEXT1"), "TEXT1 data");

    announce_elem_offset("KIERROS");
    kfile_read_byte_array(expData, kfile_read_value<u32>(refProject.fileHandle), refProject.fileHandle);
    validate_identical(expData, refProject("KIERROS"), refProject.element_size("KIERROS"), "KIERROS data");

    delete [] expData;

    return;
}

static void run_validation(const reference_container_c &refProject)
{
    validate_import(refProject);

    // Have RGEO export the imported reference data back to disk.
    kmaasto_save_track();
    kpalat_save_palat();

    validate_export(refProject);

    return;
}

int main(void)
{
    INFO(("Launching RallySportED/RGEO v.%d_%d.", INTERNAL_VERSION_MAJOR, INTERNAL_VERSION_MINOR));
    INFO(("NOTE: THIS IS AN AUTOMATED VALIDATION RUN. ASPECT TO BE VALIDATED: '%s'.", ASPECT_TO_TEST));

    try
    {
        kfile_create_directory(TEST_DIR, false);    /// Temp hack.
        reference_container_c refProject(project_file_name("DTA"), project_file_name("$FT"));

        initialize_rgeo(refProject);
        run_validation(refProject);
        release_rgeo();
    }
    catch (const char *const errstr)
    {
        fprintf(stderr, "Failed to validate '%s'. Encountered the following error: '%s'.\n", ASPECT_TO_TEST, errstr);
        goto fail;
    }
    /// The rest of the exceptions can fall through.

    success:
    printf("Successfully validated: '%s'.\n", ASPECT_TO_TEST);
    return EXIT_SUCCESS;

    fail:
    return EXIT_FAILURE;
}

// Flesh out some functions RGEO expects but which we don't need for this test.
bool kmain_program_is_exiting()
{
    return false;
}
void kmain_request_program_exit(int)
{
    return;
}
