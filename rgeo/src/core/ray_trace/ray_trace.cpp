/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED ray-tracing
 *
 * Uses the Moller-Trumbore ray-triangle intersection algorithm to serve mouse-
 * picking needs.
 *
 */

#include "../../core/ui/interactible.h"
#include "../../core/ray_trace.h"
#include "../../core/texture.h"
#include "../../core/geometry.h"

/*
 * TODOS:
 *
 * - uv coordinates from the ray intersector are wrong. whether this is a problem
 *   with the intersect implementation or the uvs themselves is to be seen.
 *
 */

struct ray_s
{
    vector3<real> orig, dir;
};

static const real EPSILON = 0.000001;

static vector3<real> vec_cross_product(const vector3<real> *const a, const vector3<real> *const b)
{
    vector3<real> cross;

    cross.x = (a->y * b->z) - (a->z * b->y);
    cross.y = (a->z * b->x) - (a->x * b->z);
    cross.z = (a->x * b->y) - (a->y * b->x);

    return cross;
}

static real vec_dot_product(const vector3<real> *const a, const vector3<real> *const b)
{
    return ((a->x * b->x) + (a->y * b->y) + (a->z * b->z));
}

// Adapted from Moller & Trumbore 2005: "Fast, minimum storage ray/triangle intersection".
//
// Intersects the given ray against the given triangle. Returns true if an intersection
// occurs and is closer than the given distance; otherwise returns false. Modifies the
// given distance to equal the new intersection distance if a nearer intersection
// does occur.
//
static bool ray_intersects_triangle_closest(const ray_s &r, const triangle_s &t,
                                            real *const closestSoFar)
{
    real det, invD, distance, u, v;
    vector3<real> e1, e2, tv, pv, qv;

    // If interaction with this triangle is set to none/ignored, assume the user
    // doesn't want it mouse-picked.
    if (t.interact.type == INTERACTIBLE_IGNORE)
    {
        return false;
    }

    e1.x = t.v[1].x - t.v[0].x;
    e1.y = t.v[1].y - t.v[0].y;
    e1.z = t.v[1].z - t.v[0].z;

    e2.x = t.v[2].x - t.v[0].x;
    e2.y = t.v[2].y - t.v[0].y;
    e2.z = t.v[2].z - t.v[0].z;

    pv = vec_cross_product(&r.dir, &e2);
    det = vec_dot_product(&e1, &pv);
    if ((det > -EPSILON) && (det < EPSILON))
    {
        return false;
    }

    invD = 1.0 / det;
    tv.x = r.orig.x - t.v[0].x;
    tv.y = r.orig.y - t.v[0].y;
    tv.z = r.orig.z - t.v[0].z;
    u = vec_dot_product(&tv, &pv) * invD;
    if ((u < 0) || (u > 1))
    {
        return false;
    }

    qv = vec_cross_product(&tv, &e1);
    v = vec_dot_product(&r.dir, &qv) * invD;
    if ((v < 0) || ((u + v) > 1))
    {
        return false;
    }

    distance = vec_dot_product(&e2, &qv) * invD;
    if ((distance < *closestSoFar) && (distance > 0))
    {
        *closestSoFar = distance;
    }
    else
    {
        return false;
    }

    return true;
}

// Returns the index in the given mesh of the triangle closest to a ray cast toward
// the given screen coordinates. Note that the mesh must be transformed to screen-
// space before calling this function.
int kuil_ray_closest_tri_under_screen_coords(const std::vector<triangle_s> &screenspaceMesh,
                                             const uint screenX, const uint screenY)
{
    int closestIdx = RAYTRACE_NO_HIT;
    real closestHit = 9999999;
    const ray_s ray = {{(real)screenX, (real)screenY, -100}, // Origin. Start at a negative z offset, so we don't miss UI elements that're at about 0.
                       {0, 0, 1}};                           // Direction (straight forward into the screen, since the mesh is in screen-space).

    for (uint i = 0; i < screenspaceMesh.size(); i++)
    {
        if (ray_intersects_triangle_closest(ray, screenspaceMesh[i], &closestHit))
        {
            closestIdx = i;
        }
    }

    return closestIdx;
}
