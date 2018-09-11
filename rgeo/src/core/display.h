/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "common.h"
#include "types.h"
#include "ui.h"

void kd_acquire_display(void);

void kd_release_display(void);

void kd_update_display(void);

void kd_target_fps(const uint fps);

uint kd_current_fps(void);

void kd_show_headless_assert_error_message(const char *const msg);

void kd_show_headless_info_message(const char *const msg);

void kd_show_headless_warning_message(const char *const msg);

resolution_s kd_display_resolution(void);

vector2<real> kd_cursor_position(void);

void kd_set_display_palette(const color_rgb_s *const p);

#endif
