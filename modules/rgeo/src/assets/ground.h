/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: RallySportED-DOS / RGEO
 * 
 */

#ifndef GROUND_H
#define GROUND_H

int kground_width(void);

int kground_height(void);

// Returns the meshes that consitute the ground view.
const struct kelpo_generic_stack_s* kground_ground_view(void);

// (Re-)generate the meshes that constitute the ground view.
void kground_regenerate_ground_view(void);

// Set the XY track tile position from which to generate the ground view.
void kground_set_ground_view_offset(const int x, const int y);

void kground_initialize_ground(const unsigned groundIdx);

void kground_release_ground(void);

#endif
