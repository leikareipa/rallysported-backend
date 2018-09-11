/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED null display
 *
 */

#include "../../../core/geometry.h"
#include "../../../core/common.h"

vector2<real> kd_cursor_position(void)
{
    return {0, 0};
}

resolution_s kd_display_resolution(void)
{
    return {0, 0, 0};
}

uint kd_current_fps(void)
{
    return 1;
}

void kd_release_display()
{
    return;
}

void kd_update_display(void)
{
    return;
}

void kd_target_fps(const uint fps)
{
    return;
}

void kd_set_display_palette(const color_rgb_s *const p)
{
    return;
}

void kd_acquire_display(void)
{
    return;
}

void kd_show_headless_info_message(const char *const msg)
{
	printf("A message from RallySportED: '%s'\n", msg);

    return;
}

void kd_show_headless_warning_message(const char *const msg)
{
	printf("A warning from RallySportED: '%s'\n", msg);

    return;
}

void kd_show_headless_assert_error_message(const char *const msg)
{
	printf("RallySportED assertion fail: '%s'\n", msg);

    return;
}
