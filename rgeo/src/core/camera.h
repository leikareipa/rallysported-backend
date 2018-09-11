/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "../core/geometry.h"

void kcamera_set_position(const int x, const int y, const int z);

void kcamera_move(const int deltaX, const int deltaY, const int deltaZ);

const vector3<int> *kcamera_position(void);

void kcamera_reset_camera_movement(void);

const vector3<int>* kcamera_direction(void);

matrix44_s kcamera_position_matrix(void);

matrix44_s kcamera_direction_matrix(void);

bool kcamera_camera_is_moving(void);

void kcamera_toggle_view_mode(void);

bool kcamera_is_topdown(void);

const vector3<int> *kcamera_camera_speed(void);

const vector3<int> *kcamera_initial_position(void);

int kcamera_tile_position_x(void);

int kcamera_tile_position_y(void);

#endif
