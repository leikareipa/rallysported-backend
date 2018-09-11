/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include "../core/ui/interactible.h"
#include "../core/common.h"
#include "../core/types.h"

struct frame_buffer_s;
struct triangle_s;
class texture_c;

template <typename T>
struct vector2
{
    T x, y;
};

template <typename T>
struct vector3
{
    T x, y, z;
};

struct vertex4_s
{
    fixedpoint_t x, y, z, w;
    real uv[2]; /// TODO. Go for fixed-point.
};

struct triangle_s
{
    vertex4_s v[3];

    // If the texture pointer is NULL, the baseColor will be used instead when
    // rendering this triangle.
    const texture_c *texturePtr;

    // Set to 255 to enable transparency of palette index 0 on the texture.
    palette_idx_t baseColor;

    interactible_s interact;
};

struct matrix44_s
{
    real data[4*4] = {0};
};

void kg_initialize_geometry(void);

void kg_release_geometry(void);

void kg_transform_mesh_to_screen_space(const std::vector<triangle_s> &srcMesh, std::vector<triangle_s> *const targetMesh);

#endif
