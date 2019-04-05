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

/* Copies the contents of the given file into the project's file.*/
void copy_to_project_file(const char *const srcFile, const file_handle projFileHandle)
{
    const file_handle srcHandle = kf_open_file(srcFile, "rb");

    kf_buffered_data_copy(srcHandle, projFileHandle);

    kf_close_file(srcHandle);

    return;
}

/* The project file consists of the given track's asset files concatenated into
 * one file. This function creates such a file of the given track, and returns
 * its file handle.*/
file_handle kproj_create_project_file_for_track(const uint trackIdx, const char *const projectName)
{
    file_handle projFileHandle = KF_INVALID_HANDLE;

    if (!kproj_is_valid_project_name(projectName))
    {
        ERROR(("Attempted to create a project with an invalid name ('%s').", projectName));
        return KF_INVALID_HANDLE;
    }

    if (trackIdx >= KEXE_NUM_TRACKS)
    {
        ERROR(("Trying to create a new project with a track index out of bounds (%d).", trackIdx));
        return KF_INVALID_HANDLE;
    }

    /* The sprintf()s below expect single-character track indices.*/
    k_assert((trackIdx < 10), "Expected a single-digit track index.");

    /* Create the project file.*/
    {
        char filename[MAX_PROJECT_NAME_LEN*2 + 2];

        /* Create the project's directory.*/
        /* TODO: Check if the dir already exists, and poll the user on whether
         * they want the existing contents overwritten in that case.*/
        kf_create_directory(projectName);

        /* Create the project file into its directory.*/
        sprintf(filename, "%s/%s.DTA", projectName, projectName);
        projFileHandle = kf_open_file(filename, "wb");

        /* Copy asset files' contents into the project file.*/
        sprintf(filename, "MAASTO.00%c", ('0' + trackIdx));
        copy_to_project_file(filename, projFileHandle);

        /* TODO: The rest of the files are copied here...*/

        /* TODO: The rest of the files are copied here...*/

        /* TODO: The rest of the files are copied here...*/
    }

    return projFileHandle;
}
