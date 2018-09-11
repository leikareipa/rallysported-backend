/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef RAY_TRACE_H
#define RAY_TRACE_H

#include "types.h"
#include "geometry.h"

int kuil_ray_closest_tri_under_screen_coords(const std::vector<triangle_s> &screenspaceMesh, const uint screenX, const uint screenY);

#endif
