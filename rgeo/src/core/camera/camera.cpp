/*
 * 2016-2018 Tarpeeksi Hyvae Soft /
 * RallySportED camera
 *
 * Used as an aid in translating triangles in world-coordinates onto screen-
 * coordinates based on where in the world the user would like to look at.
 *
 */

#include "../../core/geometry/geometry_sw.h"
#include "../../core/geometry.h"
#include "../../core/fixedpt.h"
#include "../../core/maasto.h"
#include "../../core/camera.h"
#include "../../core/ui.h"
#include "../../core/types.h"

/*
 * TODOS:
 *
 * - make a small animation for when the camera switches between topdown and
 *   normal view.
 *
 */

static const vector3<int> INITIAL_CAMERA_POS = {-1664, -760, 4400};///y -680
static const vector3<int> INITIAL_CAMERA_DIR = {-2500, 0, 0};///-x 1968

static vector3<int> CAMERA_POS = INITIAL_CAMERA_POS;
static vector3<int> CAMERA_DIR = INITIAL_CAMERA_DIR;
static vector3<int> CAMERA_SPEED = {0, 0, 0};           // The speed at which the camera is moving at a particular moment.

static bool IS_TOPDOWN = false;     // Set to true if the camera is currently using a top-down view instead of the normal Rally-Sport one.

void kcamera_set_position(const int x, const int y, const int z)
{
    CAMERA_POS.x = x;
    CAMERA_POS.y = y;
    CAMERA_POS.z = z;

    return;
}

void kcamera_toggle_view_mode(void)
{
    /// For prototyping the feature of smooth camera movement up/down.
    //CAMERA_POS.y += -40;
    //CAMERA_DIR.x -= FP_ROT_DEG(1);
    //return;

    IS_TOPDOWN = !IS_TOPDOWN;

    if (IS_TOPDOWN)
    {
        CAMERA_POS.y = -2000;
        CAMERA_DIR.x = -FP_ROT_DEG(40);
    }
    else
    {
        CAMERA_POS.y = INITIAL_CAMERA_POS.y;
        CAMERA_DIR.x = INITIAL_CAMERA_DIR.x;
    }

    return;
}

void kcamera_move(const int deltaX, const int deltaY, const int deltaZ)
{
    const vector3<int> prevPos = CAMERA_POS;
    const uint visibleWidth = kmaasto_track_tile_width() * kmaasto_track_width();
    const uint visibleHeight = kmaasto_track_tile_height() * kmaasto_track_height();
    const int maxCamPosX = int(-(visibleWidth) + ((kmaasto_maasto_draw_tile_width()) * kmaasto_track_tile_width())) - int(kmaasto_track_tile_width())/4;
    const int maxCamPosZ = int(visibleHeight) - ((kmaasto_maasto_draw_tile_height()-4) * kmaasto_track_tile_height());
    const int minCamPosX = (int(kmaasto_track_tile_width() + int(kmaasto_track_tile_width())/8));
    const int minCamPosZ = 0;

    CAMERA_POS.x -= deltaX;
    CAMERA_POS.y += deltaY;
    CAMERA_POS.z -= deltaZ;

    // Bounds-check the camera.
    if (CAMERA_POS.x > minCamPosX)
    {
        CAMERA_POS.x = minCamPosX;
    }
    else if (CAMERA_POS.x < maxCamPosX)
    {
        CAMERA_POS.x = maxCamPosX;
    }

    if (CAMERA_POS.z < minCamPosZ)
    {
        CAMERA_POS.z = minCamPosZ;
    }
    else if (CAMERA_POS.z > maxCamPosZ)
    {
        CAMERA_POS.z = maxCamPosZ;
    }

    CAMERA_SPEED.x += (prevPos.x - CAMERA_POS.x);
    CAMERA_SPEED.y += (CAMERA_POS.y - prevPos.y);
    CAMERA_SPEED.z += (CAMERA_POS.z - prevPos.z);

    return;
}

bool kcamera_is_topdown(void)
{
    return IS_TOPDOWN;
}

const vector3<int>* kcamera_position(void)
{
    return &CAMERA_POS;
}

const vector3<int>* kcamera_direction(void)
{
    return &CAMERA_DIR;
}

int kcamera_tile_position_x(void)
{
    return -CAMERA_POS.x / int(kmaasto_track_tile_width());
}

int kcamera_tile_position_y(void)
{
    return (CAMERA_POS.z / int(kmaasto_track_tile_height()));
}

/// TODO. A bit computationally wasteful.
matrix44_s kcamera_position_matrix(void)
{
    matrix44_s pm;

    g_make_transl_mat(&pm, CAMERA_POS.x, CAMERA_POS.y, CAMERA_POS.z - (IS_TOPDOWN? 150 : 40));

    return pm;
}

/// TODO. A bit computationally wasteful.
matrix44_s kcamera_direction_matrix(void)
{
    matrix44_s dm;

    g_make_rot_mat(&dm, CAMERA_DIR.x, CAMERA_DIR.y, CAMERA_DIR.z);

    return dm;
}

void kcamera_reset_camera_movement(void)
{
    CAMERA_SPEED = {0, 0, 0};

    return;
}

const vector3<int>* kcamera_initial_position(void)
{
    return &INITIAL_CAMERA_POS;
}

bool kcamera_camera_is_moving(void)
{
    return bool(CAMERA_SPEED.x || CAMERA_SPEED.y || CAMERA_SPEED.z);
}

const vector3<int>* kcamera_camera_speed(void)
{
    return &CAMERA_SPEED;
}
