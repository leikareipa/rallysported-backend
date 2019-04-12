#ifndef PROJECT_H_
#define PROJECT_H_

#include "types.h"

int kproj_test(void);
void kproj_load_data_of_project(const char *const projName);
int kproj_is_valid_project_name(const char *const name);
int kproj_create_project_for_track(const uint trackIdx, const char *const projectName);

#endif
