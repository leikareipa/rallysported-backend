/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Handles things to do with RallySportED projects, like creating and
 * loading project files.
 * 
 */

#include <string.h>
#include "project.h"
#include "common.h"

/* Returns 1 if the given name is valid; otherwise returns 0.
 * In general, project names can have up to eight characters, each of which
 * must be in the range A-Z (uppercase only).*/
int kproj_is_valid_project_name(const char *const name)
{
    uint i;
    const uint nameLength = strlen(name);

    if (nameLength > 8) return 0;
    
    for (i = 0; i < nameLength; i++)
    {
        if (name[i] < 'A' || name[i] > 'Z') return 0;
    }

    return 1;
}

void kproj_create_project_file_for_track(const u8 trackIdx, const char *const projectName)
{
    k_assert(kproj_is_valid_project_name(projectName), "Invalid project name.");

    return;
}
