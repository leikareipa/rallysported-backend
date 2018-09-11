/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef GEOMETRY_PC_H
#define GEOMETRY_PC_H

#include "../geometry.h"

void g_tri_perspective_divide(triangle_s *const t);

u16 g_tri_is_facing_camera(const triangle_s *const t);

void g_transform_vert(vertex4_s *const v, const matrix44_s *const m);

void g_mul_two_mats(const matrix44_s *const m1, const matrix44_s *const m2, matrix44_s *const dstMat);

void g_make_rot_mat(matrix44_s *const dstMat, u16 x, u16 y, u16 z);

void g_make_transl_mat(matrix44_s *const dstMat, const i32 x, const i32 y, const i32 z);

void g_make_persp_mat(matrix44_s *const dstMat, const real fov, const real aspectRatio, const i32 zNear, const i32 zFar);

void g_make_scaling_mat(matrix44_s *const dstMat, const i32 x, const i32 y, const i32 z);

void g_make_screen_space_mat(matrix44_s *const dstMat, const real halfWidth, const real halfHeight);

#endif
