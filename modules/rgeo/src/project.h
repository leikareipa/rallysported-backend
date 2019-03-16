#ifndef PROJECT_H_
#define PROJECT_H_

#include "types.h"

int kproj_is_valid_project_name(const char *const name);

void kproj_create_project_file_for_track(const u8 trackIdx, const char *const projectName);

#endif
