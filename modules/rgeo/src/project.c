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
#include "common.h"
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
        file_handle projFileHandle = KF_INVALID_HANDLE;
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
            /* Create the project file into its directory.*/
            /* TODO: A forward slash as a dir separator is likely to fail in
             *       real DOS, though it works in DOSBox on Linux.*/
            sprintf(filename, "%s/%s.DTA", projectName, projectName);
            projFileHandle = kf_open_file(filename, "wb");

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
            #include "hitable.inc"

            file_handle fh = KF_INVALID_HANDLE;

            /* TODO: A forward slash as a dir separator is likely to fail in
             *       real DOS, though it works in DOSBox on Linux.*/
            sprintf(filename, "%s/HITABLE.TXT", projectName);

            fh = kf_open_file(filename, "wb");
            kf_write_bytes(HITABLE_TXT, HITABLE_TXT_LEN, fh);
            kf_close_file(fh);
        }

        kf_close_file(projFileHandle);
    }

    return 1;
}
