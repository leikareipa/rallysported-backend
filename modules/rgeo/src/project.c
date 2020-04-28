/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Handles things to do with RallySportED projects, like creating and
 * loading project files.
 * 
 */

#include <string.h>
#include <stdio.h>
#include "exe_info.h"
#include "project.h"
#include "varimaa.h"
#include "maasto.h"
#include "common/globals.h"
#include "palat.h"
#include "file.h"

#define MAX_PROJECT_NAME_LEN 8

/* Returns 1 if the given name is valid; otherwise returns 0.
 * In general, project names can have up to eight characters, each of which
 * must be in the range A-Z (uppercase only).*/
int kproj_is_valid_project_name(const char *const name)
{
    uint i;
    const uint nameLength = strlen(name);

    if (nameLength > MAX_PROJECT_NAME_LEN) return 0;
    
    for (i = 0; i < nameLength; i++)
    {
        if (name[i] < 'A' || name[i] > 'Z') return 0;
    }

    return 1;
}

/* Creates a string pointing to the given project's project file. Expects that
 * projects' folders are directly above the current directory.*/
const char* project_file_name(const char *const projName)
{
    static char filename[MAX_PROJECT_NAME_LEN*2 + 2];

    /* TODO: A forward slash as a dir separator is likely to fail in real DOS,
     *       though it works in DOSBox on Linux.*/
    sprintf(filename, "%s/%s.DTA", projName, projName);

    return filename;
}

/* Loads the data (MAASTO, VARIMAA, and PALAT) of the project whose name is
 * given. Triggers an assertion failure if unsuccessful.*/
void kproj_load_data_of_project(const char *const projName)
{
    k_assert(kproj_is_valid_project_name(projName), "Was asked to load a project with an invalid name.");

    {
        const file_handle projFileHandle = kf_open_file(project_file_name(projName), "rb");

        km_load_maasto_data(projFileHandle);
        kv_load_varimaa_data(projFileHandle);
        kp_load_palat_data(projFileHandle);

        kf_close_file(projFileHandle);
    }

    return;
}

/* Copies the contents of the given file into the given project file, prepending
 * the data with 4 bytes describing the source file's byte length.*/
void copy_to_project_file(const char *const srcFile, const file_handle projFileHandle)
{
    const file_handle srcHandle = kf_open_file(srcFile, "rb");
    const u32 srcFileSize = kf_file_size(srcHandle);

    kf_write_bytes((const u8*)&srcFileSize, 4, projFileHandle);
    kf_copy_contents(srcHandle, projFileHandle);

    kf_close_file(srcHandle);

    return;
}

/* Creates a new RallySportED project of the given name, and initializes it
 * with the assets of the given track. Returns 1 if the creation succeeded;
 * 0 or an assertion failure otherwise.
 * 
 * A RallySportED project consists of a project data file, a manifesto file,
 * and a HITABLE file. The project file is a concatenation of the track's asset
 * files; the manifesto file is a set of directives for the RallySportED loader
 * to modify certain hard-coded parameters in Rally-Sport's binaries; and the
 * HITABLE file contains Rally-Sport's default hi-scores for each track.
 * 
 * Note: The track index is expected in the range 1..n, i.e. with 1-based
 * indexing.*/
int kproj_create_project_for_track(const uint trackIdx, const char *const projectName)
{
    if (!kproj_is_valid_project_name(projectName))
    {
        ERROR(("Attempted to create a project with an invalid name ('%s').", projectName));
        return 0;
    }

    if (kf_directory_exists(projectName))
    {
        ERROR(("Trying to create a new project with a name that's already in use."));
        return 0;
    }

    if ((trackIdx < 1) || (trackIdx > KEXE_NUM_TRACKS))
    {
        ERROR(("Trying to create a new project with a track index out of bounds (%u).", trackIdx));
        return 0;
    }
    
    /* Create the project file.*/
    {
        char filename[MAX_PROJECT_NAME_LEN*2 + 2];
        const uint palatIdx = kexe_pala_index_for_track_index(trackIdx);
        
        /* The sprintf()s below expect single-character indices.*/
        k_assert((trackIdx < 10), "Expected a single-digit track index.");
        k_assert((palatIdx < 10), "Expected a single-digit PALAT index.");

        /* Create the project's directory.*/
        /* TODO: Check if the dir already exists, and poll the user on whether
         * they want the existing contents overwritten in that case.*/
        kf_create_directory(projectName);

        /* Copy asset files' contents into the project file.*/
        {
            const file_handle projFileHandle = kf_open_file(project_file_name(projectName), "wb");

            sprintf(filename, "MAASTO.00%c", ('0' + trackIdx));
            copy_to_project_file(filename, projFileHandle);

            sprintf(filename, "VARIMAA.00%c", ('0' + trackIdx));
            copy_to_project_file(filename, projFileHandle);

            sprintf(filename, "PALAT.00%c", ('0' + palatIdx));
            copy_to_project_file(filename, projFileHandle);

            copy_to_project_file("ANIMS.DTA", projFileHandle);

            copy_to_project_file("TEXT1.DTA", projFileHandle);

            sprintf(filename, "KIERROS%c.DTA", ('0' + trackIdx));
            copy_to_project_file(filename, projFileHandle);

            kf_close_file(projFileHandle);
        }

        /* Create a default manifesto file.*/
        {
            char tmpBuffer[100];
            file_handle fh = KF_INVALID_HANDLE;

            /* TODO: A forward slash as a dir separator is likely to fail in
             *       real DOS, though it works in DOSBox on Linux.*/
            sprintf(filename, "%s/%s.$FT", projectName, projectName);
            sprintf(tmpBuffer, "0 %c %c %c\n99\n", ('0' + trackIdx),
                                                   ('0' + palatIdx),
                                                   ('0' + REQUIRED_LOADER_VER));

            fh = kf_open_file(filename, "wb");
            kf_write_string(tmpBuffer, fh);
            kf_close_file(fh);
        }

        /* Create a default HITABLE.TXT file.*/
        {
            #include "assets/hitable.inc"

            file_handle fh = KF_INVALID_HANDLE;

            /* TODO: A forward slash as a dir separator is likely to fail in
             *       real DOS, though it works in DOSBox on Linux.*/
            sprintf(filename, "%s/HITABLE.TXT", projectName);

            fh = kf_open_file(filename, "wb");
            kf_write_bytes(HITABLE_TXT, HITABLE_TXT_LEN, fh);
            kf_close_file(fh);
        }
    }

    return 1;
}

/* Returns true if the unit appears to be functioning correctly, otherwise false.*/
int kproj_test(void)
{
    if (!kproj_is_valid_project_name("VALIDNAM")) return 0; /* A valid name.*/
    if (kproj_is_valid_project_name("V LIDNAM")) return 0; /* Spaces aren't allowed.*/
    if (kproj_is_valid_project_name("V4L1DN4M")) return 0; /* Numbers aren't allowed.*/
    if (kproj_is_valid_project_name("NOTAVALIDNAME")) return 0; /* Too long.*/

    if (kproj_create_project_for_track(KEXE_NUM_TRACKS+1, "YYYYYYYY")) return 0; /* Track index out of bounds.*/
    if (kproj_create_project_for_track(KEXE_NUM_TRACKS, "YYYYYYYYY")) return 0; /* Invalid track name (too long).*/

    return 1;
}
