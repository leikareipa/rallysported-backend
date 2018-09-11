/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED user interface: logic
 *
 */

#include <cstdlib>
#include <cstring>
#include <cmath>
#include "../../core/ui/interactible.h"
#include "../../core/frame_timer.h"
#include "../../core/ray_trace.h"
#include "../../core/geometry.h"
#include "../../core/texture.h"
#include "../../core/display.h"
#include "../../core/palette.h"
#include "../../core/maasto.h"
#include "../../core/camera.h"
#include "../../core/palat.h"
#include "../../core/main.h"
#include "../../core/ui.h"

/* TODOS:
 *
 * - this unit needs cleanup overall.
 *
 * - dragging props should have the prop follow the cursor in screen-space, so it
 *   looks and feels like it's actually being dragged by the cursor rather than
 *   moving independently.
 *
 * - for moving the ground up/down, have better scaling with fps.
 *
 * - the input system here depends on the ray tracer, which is currently not returning
 *   valid uv coordinates. that needs fixing, so the interactible system's implementation
 *   can be improved.
 *
 * - camera movement is a bit iffy, having to move the cursor to the edges of the
 *   screen. needs better.
 *
 * - the texture editor should have an undo function, e.g. with the right mouse button
 *   restoring a pixel to the value it had when last loaded from disk.
 *
 */

static enum { VIEW_MAIN/*3d view*/, VIEW_PAINT/*topdown 2d paint*/, VIEW_TEX/*texture edit*/ } CURRENT_VIEW;

// Toggles for various UI and UI-related furnishings.
static bool SHOW_REAL_WATER_LEVEL = false;
static bool PALAT_PANE_OPEN = false;
static bool DRAW_WIREFRAME = false;
static bool DRAW_FPS = true;

static vector3<i32> CAMERA_SPEED = {0, 0, 0};   /// Temp.

static bool CURSOR_IS_INSIDE_PALAT_PANE = false;
static bool IS_DRAGGING_PROP = false;
static bool IS_EDITING_TERRAIN = false;

// Whether the given mouse button is pressed at the moment.
static bool MOUSE_LEFT_PRESSED = false;
static bool MOUSE_RIGHT_PRESSED = false;
static bool MOUSE_MIDDLE_PRESSED = false;

// Set to true if there have been any user edits since the project was last saved.
static bool UNSAVED_CHANGES_EXIST = false;

// The brush is what we edit the terrain with - it can paint, raise/lower, smoothen, etc.
namespace brush_n
{
    static u8 COLOR = 4;
    static int SIZE = 0;
    static u8 TEXTURE_IDX = 3;  // Which PALA texture we'd paint with.
    static bool IS_SMOOTHING = false;   // If true, applying the brush on terrain will smoothen the ground under it.
}

// Which PALA the cursor is currently hovering over in the PALAT pane.
static u8 PALAT_PANE_HOVER_IDX = 0;

// Which key on the keyboard is currently being pressed.
static input_key_e CURRENT_KEYPRESS = INPUT_KEY_NONE;

static interactible_s CURRENT_MOUSE_INTERACTION;
bool LOCK_MOUSE_INTERACTION = false;

// The location of the mouse cursor.
static vector3<i32> CURSOR_POS = {0, 0, 0};            // Relative to the window.
static vector3<i32> CURSOR_POS_NATIVE = {0, 0, 0};     // Relative to the window but in the game's native coordinates.
static vector3<i32> CURSOR_POS_PERCENTAGE = {0, 0, 0};   // Relative to the window in percentages (0..100).


static void process_mouse_movement(void)
{
    static vector3<i32> prevMousePos = CURSOR_POS_NATIVE;

    // Reset certain variables to their baseline values. They'll be modified in this
    // function if need be.
    kuil_set_cursor(kui_default_cursor());
    CURSOR_IS_INSIDE_PALAT_PANE = false;
    kcamera_reset_camera_movement();

    if (kui_is_brush_smoothing())
    {
        kuil_set_cursor(CURSOR_ARROWSMOOTHING);
    }

    if (CURRENT_MOUSE_INTERACTION.type == INTERACTIBLE_PALATPANE)
    {
        CURSOR_IS_INSIDE_PALAT_PANE = true;
    }

    // If the PALAT pane (texture atlas) is open.
    if (kui_palat_pane_is_open() &&
        CURSOR_IS_INSIDE_PALAT_PANE)
    {
        const real resMulX = NATIVE_RES_MUL_X;
        const real resMulY = NATIVE_RES_MUL_X;
        const real ratio = 1;//1 + (resMulY - floor(resMulY));
        const uint palaX = CURSOR_POS.x / ((8*(resMulX)) * ratio);  /// TODO. Get these values integrated with where the renderer puts these elements on the screen.
        const uint palaY = (CURSOR_POS.y) / (8 * resMulY);

        PALAT_PANE_HOVER_IDX = palaX + palaY * kuil_palat_pane_num_x();
    }

    if (MOUSE_LEFT_PRESSED || MOUSE_RIGHT_PRESSED)
    {
        if (CURRENT_MOUSE_INTERACTION.type == INTERACTIBLE_PROP &&
            !IS_EDITING_TERRAIN)
        {
            IS_DRAGGING_PROP = true;
        }
        else
        {
            IS_DRAGGING_PROP = false;
        }

        if (CURRENT_MOUSE_INTERACTION.type == INTERACTIBLE_GROUND)
        {
            IS_EDITING_TERRAIN = true;
        }
        else
        {
            IS_EDITING_TERRAIN = false;
        }
    }
    else
    {
        IS_DRAGGING_PROP = false;
        IS_EDITING_TERRAIN = false;
    }

    // Move a prop.
    if (IS_DRAGGING_PROP)
    {
        vector3<int> delta = {(CURSOR_POS_NATIVE.x - prevMousePos.x)*5,
                               0,//CURSOR_POS.x - prevMousePos.y,
                               (CURSOR_POS_NATIVE.y - prevMousePos.y)*16};

        if (!kmaasto_move_prop(CURRENT_MOUSE_INTERACTION.params[INTERACT_PARAM_PROP_IDX], delta))
        {
            kuil_set_cursor(CURSOR_NOTALLOWED);
        }
        else
        {
            kuil_set_cursor(CURSOR_HANDGRAB);
        }

        if (kui_current_keypress() == INPUT_KEY_PANE)
        {
            kmaasto_change_prop_type(CURRENT_MOUSE_INTERACTION.params[INTERACT_PARAM_PROP_IDX]);
        }
    }
    else if (CURRENT_MOUSE_INTERACTION.type == INTERACTIBLE_PROP)
    {
        kuil_set_cursor(CURSOR_HAND);
    }

    // Move the camera based on where the mouse cursor is.
    if (!CURSOR_IS_INSIDE_PALAT_PANE &&
        !kui_paint_view_is_open() &&
        !kui_texedit_view_is_open())
    {
        const uint msSinceLastFrame = kftimer_elapsed();
        const real moveSpeedX = 1.4 * (msSinceLastFrame > 1000? 0 : msSinceLastFrame);    // <- Try to prevent overflow of the movement by clipping the timer.
        const real moveSpeedY = (kcamera_is_topdown()? 1.4 : 2.4) * (msSinceLastFrame > 1000? 0 : msSinceLastFrame);
        const int screenMargin = 7;

        // If the cursor is at the edges of the screen, move the camera in the
        // cursor's direction.
        if (kui_cursor_pos_native().x < screenMargin ||
            kui_cursor_pos_native().y < (screenMargin*2) || /// *2 for convenience, so you don't need to move the cursor so far up.
            kui_cursor_pos_native().x >= int(NATIVE_RESOLUTION.w - screenMargin) ||
            kui_cursor_pos_native().y >= int(NATIVE_RESOLUTION.h - screenMargin*2)) /// *2 to kludge-fix issue in fullscreen where the cursor can't move far enough down to scroll the camera.
        {
            // Get a normal pointing from the middle of the screen toward the cursor.
            vector3<real> normal;
            normal = {real(CURSOR_POS_PERCENTAGE.x - 50), real(50 - CURSOR_POS_PERCENTAGE.y), 0};
            const real w = sqrt((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z));
            normal.x = (normal.x / w) * moveSpeedX;
            normal.y = (normal.y / w) * moveSpeedY;

            static vector3<real> frac = {0, 0, 0};  // The camera moves by integers, so keep track of the leftover fractions for better movement across frames.
            frac.x += normal.x;
            frac.z += normal.y;

            vector3<int> moveAmt = {(int)frac.x, 0, (int)frac.z};
            kcamera_move(moveAmt.x, 0, moveAmt.z);  // The camera isn't expected to move on y.

            frac.x -= moveAmt.x;
            frac.z -= moveAmt.z;
        }
    }

    // If we're holding on to an prop while moving the camera, have the
    // prop move with us.
    if (IS_DRAGGING_PROP &&
        kcamera_camera_is_moving())
    {
        kmaasto_move_prop(CURRENT_MOUSE_INTERACTION.params[INTERACT_PARAM_PROP_IDX], CAMERA_SPEED);
    }

    prevMousePos = CURSOR_POS_NATIVE;

    return;
}

void process_mouse_clicks(void)
{
    if (!(MOUSE_MIDDLE_PRESSED | MOUSE_LEFT_PRESSED | MOUSE_RIGHT_PRESSED))
    {
        LOCK_MOUSE_INTERACTION = false;   // No buttons are being held, so we can release.

        return;
    }

    // The interaction lock is used to make sure that holding a mouse button while
    // moving the cursor won't suddenly register a different kind of interaction when
    // the cursor moves over another interactible. The lock will keep the focus of
    // interaction on whatever type of interaction was current when the button was
    // pressed down, releasing the lock once the button has been let go.
    LOCK_MOUSE_INTERACTION = true;

    // If the PALA pane is open, interpret clicks inside of it as selecting the
    // current PALA.
    if (CURSOR_IS_INSIDE_PALAT_PANE ||
        (CURRENT_VIEW == VIEW_TEX)) // The texture edit view always has the PALA pane open.
    {
        /// TODO. We're supposed to pre-compute this stuff in process_mouse_movement()
        /// using mouse-picked uv coordinates, but that doesn't work for the texture-
        /// editing view yet. So we'll just re-compute here for now.
        const real ratio = 1;//1 + ((kd_display_resolution().h / (real)NATIVE_RESOLUTION.h) - floor(kd_display_resolution().h / (real)NATIVE_RESOLUTION.h));
        const int palaPaneWidth = (kuil_palat_pane_num_x() * (8*NATIVE_RES_MUL_X)) * ratio;
        const int palaPaneHeight = (kuil_palat_pane_num_y() * (8*NATIVE_RES_MUL_X));

        // If the cursor is inside the PALAT pane.
        if (CURSOR_POS.x < palaPaneWidth &&
            CURSOR_POS.y < palaPaneHeight)
        {
            const uint palaX = CURSOR_POS.x / ((8*(NATIVE_RES_MUL_X)) * ratio);
            const uint palaY = (CURSOR_POS.y) / (8 * NATIVE_RES_MUL_X);
            const uint palaIdx = palaX + palaY * kuil_palat_pane_num_x();

            if (palaIdx < 251)
            {
                brush_n::TEXTURE_IDX = palaIdx;
            }

            return;
        }

        /// This is about what we're supposed to do, once the mouse-picking works.
        /*if (PALAT_PANE_HOVER_IDX < 251)
        {
            brush_n::TEXTURE_IDX = PALAT_PANE_HOVER_IDX;
        }*/
    }

    if (CURRENT_VIEW == VIEW_MAIN)
    {
        if (CURRENT_MOUSE_INTERACTION.type == INTERACTIBLE_GROUND)
        {
            const uint msSinceLastFrame = kftimer_elapsed();

            // Paint the ground.
            if (MOUSE_MIDDLE_PRESSED)
            {
                kmaasto_set_tile_texture(CURRENT_MOUSE_INTERACTION.params[INTERACT_PARAM_GROUND_X_GLOBAL],
                                         CURRENT_MOUSE_INTERACTION.params[INTERACT_PARAM_GROUND_Z_GLOBAL],
                                         brush_n::TEXTURE_IDX, brush_n::SIZE);
            }
            // Move the ground up/down.
            else
            {
                int dir = 0;

                // The amount by which to move the ground is set to depend on the frame
                // timer to decouple it from the FPS, but since the heightmap operates
                // on integers, we also need to keep track of the fractional part across
                // frames or high FPS will result in the ground not moving at all.
                static real fracAmt = 0;
                fracAmt += std::min((0.05 * msSinceLastFrame), 10.0);
                int amt = fracAmt;
                fracAmt -= amt;

                if (MOUSE_LEFT_PRESSED)
                {
                    dir = 1;
                }
                else if (MOUSE_RIGHT_PRESSED)
                {
                    dir = -1;
                }

                kmaasto_change_tile_height(CURRENT_MOUSE_INTERACTION.params[INTERACT_PARAM_GROUND_X_GLOBAL],
                                           CURRENT_MOUSE_INTERACTION.params[INTERACT_PARAM_GROUND_Z_GLOBAL],
                                           dir * amt, brush_n::SIZE);
            }
        }

        return;
    }
    else if (CURRENT_VIEW == VIEW_PAINT)
    {
        const uint baseSize = 128;
        const uint sizeMul = baseSize / kmaasto_track_width();  /// <- Assumes the track is square.
        const uint posX = (kd_display_resolution().w - (baseSize * NATIVE_RES_MUL_X * 2)) / 2;
        const uint posY = (kd_display_resolution().h - (baseSize * NATIVE_RES_MUL_Y)) / 2;

        // The relative click position in the texture.
        const uint relX = ((CURSOR_POS.x - posX) / (NATIVE_RES_MUL_X * 2)) / sizeMul;
        const uint relY = ((CURSOR_POS.y - posY) / NATIVE_RES_MUL_Y) / sizeMul;

        if (relX < baseSize &&
            relY < baseSize)
        {
            kmaasto_set_tile_texture(relX, relY, brush_n::TEXTURE_IDX, brush_n::SIZE);
        }

        return;
    }
    else if (CURRENT_VIEW == VIEW_TEX)
    {
        const uint baseSize = kpalat_pala_width();
        const uint sizeMul = 8; // The x by which the PALA has been enlarged.
        const uint palaPosX = (kd_display_resolution().w - (baseSize * sizeMul * NATIVE_RES_MUL_X) + kuil_palat_pane_width()/2) / 2;
        const uint palaPosY = (kd_display_resolution().h - (baseSize * sizeMul * NATIVE_RES_MUL_Y)) / 2;

        // The relative click position inside the PALA we're editing.
        const uint palaClickX = ((CURSOR_POS.x - palaPosX) / NATIVE_RES_MUL_X) / sizeMul;
        const uint palaClickY = kpalat_pala_height() - (((CURSOR_POS.y - palaPosY) / NATIVE_RES_MUL_Y) / sizeMul);    /// <- Flip the y coordinate.

        // If the click is inside the texture.
        if (palaClickX < kpalat_pala_width() &&
            palaClickY < kpalat_pala_height())
        {
            kpalat_set_pala_pixel(kui_brush_pala_idx(), palaClickX, palaClickY, brush_n::COLOR);
        }
        // See if the click is inside the palette selector, and if it is, select the new color.
        else if (CURSOR_POS_NATIVE.x >= 280)
        {
            const uint colorIdx = CURSOR_POS.y / (kd_display_resolution().h / (real)kpal_num_primary_colors());

            if (colorIdx < kpal_num_primary_colors())
            {
                brush_n::COLOR = colorIdx;
            }
        }

        return;
    }

    return;
}

void kui_set_brush_size(const u8 s)
{
    brush_n::SIZE = s;

    return;
}

void process_keyboard_interaction(void)
{
    switch (CURRENT_KEYPRESS)
    {
        case INPUT_KEY_PANE:
        {
            if (!IS_DRAGGING_PROP)
            {
                PALAT_PANE_OPEN = !PALAT_PANE_OPEN;
            }

            break;
        }
        case INPUT_KEY_PAINT:
        {
            CURRENT_VIEW = (CURRENT_VIEW == VIEW_PAINT? VIEW_MAIN : VIEW_PAINT);

            break;
        }
        case INPUT_KEY_CAMERA:
        {
            kcamera_toggle_view_mode();

            break;
        }
        case INPUT_KEY_PALA:
        {
            CURRENT_VIEW = (CURRENT_VIEW == VIEW_TEX? VIEW_MAIN : VIEW_TEX);

            break;
        }
        case INPUT_KEY_REFRESH:
        {
            kmaasto_generate_topdown_view();
            kuil_update_palat_pane_texture();

            break;
        }
        case INPUT_KEY_ESC:
        {
            CURRENT_VIEW = VIEW_MAIN;

            break;
        }
        case INPUT_KEY_WIREFRAME:
        {
            DRAW_WIREFRAME = !DRAW_WIREFRAME;

            break;
        }
        case INPUT_KEY_FPS:
        {
            DRAW_FPS = !DRAW_FPS;

            break;
        }
        case INPUT_KEY_SPACE:
        {
            brush_n::IS_SMOOTHING = !brush_n::IS_SMOOTHING;

            break;
        }
        case INPUT_KEY_WATER:
        {
            SHOW_REAL_WATER_LEVEL = !SHOW_REAL_WATER_LEVEL;

            break;
        }
        case INPUT_KEY_1:
        {
            kui_set_brush_size(0);

            break;
        }
        case INPUT_KEY_2:
        {
            kui_set_brush_size(1);

            break;
        }
        case INPUT_KEY_3:
        {
            kui_set_brush_size(2);

            break;
        }
        case INPUT_KEY_4:
        {
            kui_set_brush_size(3);

            break;
        }
        case INPUT_KEY_5:
        {
            kui_set_brush_size(8);

            break;
        }
        case INPUT_KEY_SAVE:
        {
            kmaasto_save_track();
            kpalat_save_palat();

            kui_clear_unsaved_changes_flag();

            DEBUG(("Track saved to project file."));

            break;
        }
        case INPUT_KEY_EXIT:
        {
            kmain_request_program_exit(EXIT_SUCCESS);

            break;
        }
        case INPUT_KEY_NONE: break;

        default: { k_assert(0, "Detected an unhandled key-press."); break; }
    }


    return;
}

static void poll_input_state(void)
{
    const vector2<real> cPosAbs = kd_cursor_position();
    const vector2<real> cPosRel = {cPosAbs.x / (real)kd_display_resolution().w * 100,
                                cPosAbs.y / (real)kd_display_resolution().h * 100};

    // Mouse position.
    kui_set_cursor_pos(cPosAbs.x, cPosAbs.y,
                          cPosRel.x, cPosRel.y);

    return;
}

// Finds over which triangle in the given scene the mouse cursor is currently
// located. Based on this, we set the next potential interaction, i.e. the one
// we should process if the user clicks the mouse or otherwise requests interaction.
//
static void establish_mouse_hover(const std::vector<triangle_s> &scene)
{
    // Special case. If the use is currently dragging an object, we should ignore
    // any changes in mouse hover until the user lets go.
    if (IS_DRAGGING_PROP)
    {
        return;
    }

    const int triUnderCursor = kuil_ray_closest_tri_under_screen_coords(scene, kui_cursor_pos_x(), kui_cursor_pos_y());
    if (triUnderCursor == RAYTRACE_NO_HIT)
    {
        CURRENT_MOUSE_INTERACTION.type = INTERACTIBLE_NONE;

        return;
    }
    else if (uint(triUnderCursor) >= scene.size()) /// Intends to test for negative and positive.
    {
        k_assert(0, "Mouse-picking returned a triangle index out of bounds.");

        return;
    }

    // If there was a valid triangle under the cursor, assign its interaction
    // as the one we should process this frame.
    if (triUnderCursor >= 0)
    {
        const triangle_s &t = scene[triUnderCursor];

        // If applicable, highlight the interactible over which the cursor is hovering.
        if (t.interact.type == INTERACTIBLE_GROUND)
        {
            kmaasto_highlight_ground_tile(t.interact.params[INTERACT_PARAM_GROUND_X],
                                          t.interact.params[INTERACT_PARAM_GROUND_Z]);
        }
        else if (t.interact.type == INTERACTIBLE_PROP)
        {
            kmaasto_highlight_prop(t.interact.params[INTERACT_PARAM_PROP_IDX]);
        }

        // Special case. If we're already raising/lowering terrain, don't allow the
        // focus of interaction to change from that to e.g. picking up props.
        if (IS_EDITING_TERRAIN &&
            (t.interact.type != INTERACTIBLE_GROUND))
        {
            return;
        }

        if (LOCK_MOUSE_INTERACTION && t.interact.type != CURRENT_MOUSE_INTERACTION.type)
        {
            return;
        }

        CURRENT_MOUSE_INTERACTION = t.interact;
    }

    return;
}

// Polls and processes user input, like keyboard and mouse interaction. Receives
// as an argument the latest transformed (to screen coordinates) scene mesh, for
// mouse-picking.
//
void kui_process_user_input(const std::vector<triangle_s> &transformedScene)
{
    poll_input_state();

    establish_mouse_hover(transformedScene);
    process_mouse_movement();
    process_mouse_clicks();

    process_keyboard_interaction();
    kui_reset_keypress_info();

    return;
}

u8 kui_brush_pala_idx(void)
{
    return brush_n::TEXTURE_IDX;
}

bool kui_is_editing_terrain(void)
{
    return IS_EDITING_TERRAIN;
}

// Update the internal knowledge of the mouse position over the screen. Note that
// the coordinates should be given as percentages of the size of the screen (0-99).
//
void kui_set_cursor_pos(const i32 absX, const i32 absY,
                        const i32 relX, const i32 relY)
{
    CURSOR_POS.x = std::max(0, std::min((int)kd_display_resolution().w-1, absX));
    CURSOR_POS.y = std::max(0, std::min((int)kd_display_resolution().h-1, absY));

    CURSOR_POS_NATIVE.x = CURSOR_POS.x / NATIVE_RES_MUL_X;
    CURSOR_POS_NATIVE.y = CURSOR_POS.y / NATIVE_RES_MUL_Y;

    CURSOR_POS_PERCENTAGE.x = relX;
    CURSOR_POS_PERCENTAGE.y = relY;

    return;
}

void kui_set_mouse_button_left_pressed(const bool pressed)
{
    MOUSE_LEFT_PRESSED = pressed;

    return;
}

void kui_set_mouse_button_right_pressed(const bool pressed)
{
    MOUSE_RIGHT_PRESSED = pressed;

    return;
}

void kui_set_mouse_button_middle_pressed(const bool pressed)
{
    MOUSE_MIDDLE_PRESSED = pressed;

    return;
}

cursor_e kui_default_cursor(void)
{
    return CURSOR_ARROW;
}

void kui_initialize_ui_logic(void)
{
    /// Nothing here.

    return;
}

void kui_release_ui_logic(void)
{
    // Release cursors.
    {
        /// TODO. For now, their allocated memory is released automatically on exit
        /// by the memory manager.
    }

    return;
}

bool kui_is_real_water_level_enabled(void)
{
    return SHOW_REAL_WATER_LEVEL;
}

bool kui_is_mouse_left_pressed(void)
{
    return MOUSE_LEFT_PRESSED;
}

bool kui_is_mouse_right_pressed(void)
{
    return MOUSE_RIGHT_PRESSED;
}

bool kui_is_mouse_middle_pressed(void)
{
    return MOUSE_MIDDLE_PRESSED;
}

u8 kui_hovered_pala_idx(void)
{
    return PALAT_PANE_HOVER_IDX;
}

bool kui_cursor_is_inside_palat_pane(void)
{
    return CURSOR_IS_INSIDE_PALAT_PANE;
}

void kui_flag_unsaved_changes(void)
{
    UNSAVED_CHANGES_EXIST = true;

    return;
}

void kui_clear_unsaved_changes_flag(void)
{
    UNSAVED_CHANGES_EXIST = false;

    return;
}

bool kui_are_unsaved_changes(void)
{
    return UNSAVED_CHANGES_EXIST;
}

void kui_reset_keypress_info(void)
{
    CURRENT_KEYPRESS = INPUT_KEY_NONE;

    return;
}

input_key_e kui_current_keypress(void)
{
    return CURRENT_KEYPRESS;
}

void kui_set_key_pressed(const input_key_e key)
{
    CURRENT_KEYPRESS = key;

    return;
}

bool kui_is_wireframe_on(void)
{
    return DRAW_WIREFRAME;
}

bool kui_is_fps_display_on(void)
{
    return DRAW_FPS;
}

u8 kui_brush_color(void)
{
    return brush_n::COLOR;
}

bool kui_is_dragging_prop(void)
{
    return IS_DRAGGING_PROP;
}

interactible_s kui_current_interaction(void)
{
    return CURRENT_MOUSE_INTERACTION;
}

bool kui_paint_view_is_open(void)
{
    return (CURRENT_VIEW == VIEW_PAINT);
}

bool kui_texedit_view_is_open(void)
{
    return (CURRENT_VIEW == VIEW_TEX);
}

bool kui_palat_pane_is_open(void)
{
    return bool(CURRENT_VIEW == VIEW_TEX || PALAT_PANE_OPEN);   /// TODO. The tex view always has the pane open, but it's kludgy to have it as a special case here.
}

bool kui_is_brush_smoothing(void)
{
    return brush_n::IS_SMOOTHING;
}

int kui_brush_size(void)
{
    return (brush_n::SIZE + 1);
}

i32 kui_cursor_pos_x(void)
{
    return CURSOR_POS.x;
}

i32 kui_cursor_pos_y(void)
{
    return CURSOR_POS.y;
}

const vector3<i32>& kui_cursor_pos(void)
{
    return CURSOR_POS;
}

const vector3<i32>& kui_cursor_pos_percentage(void)
{
    return CURSOR_POS_PERCENTAGE;
}

const vector3<i32>& kui_cursor_pos_native(void)
{
    return CURSOR_POS_NATIVE;
}

const vector3<i32>& kui_cursor_pos_relative(void)
{
    return CURSOR_POS_PERCENTAGE;
}

