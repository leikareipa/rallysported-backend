/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED null renderer
 *
 */

#include "../../../core/render.h"
#include "../../../core/common.h"

const frame_buffer_s* kr_acquire_renderer(void)
{
    return NULL;
}

void kr_report_total_texture_size(void)
{
    DEBUG(("No texture data registered (null renderer)."));

    return;
}

void kr_depth_sort_mesh(std::vector<triangle_s> *const mesh)
{
	return;
}

void kr_set_depth_testing_enabled(const bool enabled)
{
    return;
}

void kr_release_renderer(void)
{
    return;
}

void kr_clear_frame(void)
{
	return;
}

void kr_upload_texture(texture_c *const tex)
{
	return;
}

void kr_re_upload_texture(const texture_c *const tex)
{
	return;
}

void kr_rasterize_mesh(const std::vector<triangle_s> &scene, const bool wireframe)
{
	return;
}

