/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "geometry.h"
#include "common.h"
#include "types.h"

enum input_key_e
{
    INPUT_KEY_NONE = 0,
    INPUT_KEY_PANE,
    INPUT_KEY_ESC,
    INPUT_KEY_WIREFRAME,
    INPUT_KEY_SPACE,
    INPUT_KEY_PAINT,
    INPUT_KEY_PALA,
    INPUT_KEY_REFRESH,
    INPUT_KEY_EXIT,
    INPUT_KEY_FPS,
    INPUT_KEY_WATER,
    INPUT_KEY_1,
    INPUT_KEY_2,
    INPUT_KEY_3,
    INPUT_KEY_4,
    INPUT_KEY_5,
    INPUT_KEY_SAVE,
    INPUT_KEY_CAMERA
};

enum cursor_e
{
    CURSOR_ARROW,
    CURSOR_ARROWSMOOTHING,
    CURSOR_NOTALLOWED,
    CURSOR_HANDGRAB,
    CURSOR_HAND,
    CURSOR_INVISIBLE,
};

const vector3<i32>& kui_cursor_pos_native(void);

const vector3<i32>& kui_cursor_pos_percentage(void);

const vector3<i32>& kui_cursor_pos(void);

const vector3<i32>& kui_cursor_pos_relative(void);

bool kui_is_mouse_left_pressed(void);

bool kui_is_real_water_level_enabled(void);

bool kui_is_mouse_right_pressed(void);

bool kui_is_mouse_middle_pressed(void);

i32 kui_cursor_pos_x(void);

i32 kui_cursor_pos_y(void);

int kui_brush_size(void);

bool kui_is_brush_smoothing(void);

bool kui_paint_view_is_open(void);

bool kui_texedit_view_is_open(void);

bool kui_palat_pane_is_open(void);

u8 kui_hovered_pala_idx(void);

bool kui_cursor_is_inside_palat_pane(void);

void kui_flag_unsaved_changes(void);

void kui_clear_unsaved_changes_flag(void);

bool kui_are_unsaved_changes(void);

void kui_reset_keypress_info(void);

input_key_e kui_current_keypress(void);

void kui_set_key_pressed(const input_key_e key);

bool kui_is_wireframe_on(void);

bool kui_is_fps_display_on(void);

bool kui_is_dragging_prop(void);

bool kui_is_editing_terrain(void);

u8 kui_brush_color(void);

void kuil_set_cursor(const cursor_e c);

cursor_e kui_default_cursor(void);

void kui_set_brush_size(const u8 s);

void kui_set_cursor_pos(const i32 absX, const i32 absY, const i32 relX, const i32 relY);

void kui_initialize_ui_logic(void);

u8 kui_brush_pala_idx(void);

interactible_s kui_current_interaction(void);

void kui_release_ui_logic(void);

void kui_process_user_input(const std::vector<triangle_s> &transformedScene);

void kui_set_mouse_button_left_pressed(const bool pressed);

void kui_set_mouse_button_right_pressed(const bool pressed);

void kui_set_mouse_button_middle_pressed(const bool pressed);

const texture_c& kuil_current_cursor_texture(void);

resolution_s kuil_ideal_display_resolution(const bool wantFullscreen);

uint kuil_palat_pane_width(void);

uint kuil_palat_pane_height(void);

uint kuil_palat_pane_num_x(void);

uint kuil_palat_pane_num_y(void);

void kuil_render_string(const char *const str, const uint x, const uint y, std::vector<triangle_s> *const scene, const bool alpha = false);

void kuil_initialize_ui_layout(void);

void kuil_update_palat_pane_texture(void);

void kuil_regenerate_palat_pane_texture(void);

void kuil_release_ui_layout(void);

void kuil_add_user_interface_tri_mesh(std::vector<triangle_s> *const scene);

#endif
