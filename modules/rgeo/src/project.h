#ifndef PROJECT_H_
#define PROJECT_H_

#include "types.h"

int kproj_is_valid_project_name(const char *const name);

file_handle kproj_create_project_file_for_track(const uint trackIdx, const char *const projectName);

#endif
