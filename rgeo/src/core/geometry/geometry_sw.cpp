/*
 * 2017, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED miscellaneous matrix stuff
 *
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include "../../core/ray_trace.h"
#include "../../core/geometry.h"
#include "../../core/fixedpt.h"
#include "../../core/display.h"
#include "../../core/maasto.h"
#include "../../core/camera.h"
#include "../../core/ui.h"
#include "geometry_sw.h"

static matrix44_s SCREEN_SPACE_MAT;
static matrix44_s PERSP_MAT;

void kg_initialize_geometry(void)
{
    #define DEG_TO_RAD(x) (((x) * 3.141592654) / 180)

    const uint nearClip = 1;
    const uint farClip = 2000;
    const resolution_s res = kd_display_resolution();

    g_make_screen_space_mat(&SCREEN_SPACE_MAT, (res.w / 2.0), (res.h / 2.0));
    g_make_persp_mat(&PERSP_MAT, DEG_TO_RAD(25), (res.w / real(res.h)), nearClip, farClip);

    #undef DEG_TO_RAD

    return;
}

void kg_release_geometry(void)
{
    /// Not used.

    return;
}

void perspective_divide(triangle_s *const t)
{
    for (uint i = 0; i < 3; i++)
    {
        t->v[i].x /= /*(real)*/t->v[i].w;
        t->v[i].y /= /*(real)*/t->v[i].w;
       // t->v[i].z /= (real)t->v[i].w;
    }

    return;
}

void kg_transform_mesh_to_screen_space(const std::vector<triangle_s> &srcMesh,
                                       std::vector<triangle_s> *const dstMesh)
{
    matrix44_s rotation, transl, groundTransform, tmp, worldSpace, clipSpace, screenSpace;
    const resolution_s displaySize = kd_display_resolution();
    const matrix44_s cameraPos = kcamera_position_matrix();
    const matrix44_s cameraDir = kcamera_direction_matrix();

    /// TODO. Get rid of having to specifically rotate the mesh to face up, i.e.
    /// sort out the coordinate system when importing.
    g_make_rot_mat(&rotation, FP_ROT_DEG(180), 0, 0);       /// Rotate to face up.
    g_make_transl_mat(&transl, 0, 0, 0);                    /// (Unused for now.)
    g_mul_two_mats(&transl, &rotation, &groundTransform);

    // Apply the camera view.
    g_mul_two_mats(&cameraPos, &groundTransform, &tmp);
    g_mul_two_mats(&cameraDir, &tmp, &worldSpace);

    // Make a matrix to transform to screen-space.
    g_mul_two_mats(&PERSP_MAT, &worldSpace, &clipSpace);
    g_mul_two_mats(&SCREEN_SPACE_MAT, &clipSpace, &screenSpace);

    // Transform the mesh to screen-space.
    for (uint i = 0; i < srcMesh.size(); i++)
    {
        triangle_s t = srcMesh[i];

        g_transform_vert(&t.v[0], &screenSpace);
        g_transform_vert(&t.v[1], &screenSpace);
        g_transform_vert(&t.v[2], &screenSpace);
        perspective_divide(&t);

        // Cull triangles that are entirely outside the screen.
        if ((t.v[0].x < 0 && t.v[1].x < 0 && t.v[2].x < 0) ||
            (t.v[0].y < 0 && t.v[1].y < 0 && t.v[2].y < 0))
        {
            continue;
        }
        if ((t.v[0].x >= (int)displaySize.w && t.v[1].x >= (int)displaySize.w && t.v[2].x >= (int)displaySize.w) ||
            (t.v[0].y >= (int)displaySize.h && t.v[1].y >= (int)displaySize.h && t.v[2].y >= (int)displaySize.h))
        {
            continue;
        }

        dstMesh->push_back(t);
    }

    return;
}
