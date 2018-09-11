/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef RENDER_H
#define RENDER_H

#include <vector>
#include "../core/common.h"

struct triangle_s;
class texture_c;

// A frame buffer, which stores a pointer to a palette and an image indexing
// into that palette.
struct frame_buffer_s
{
    color_bgra_s *canvas = nullptr; /// TODO. Use managed memory instead of a raw pointer. The current memory manager needs to get dynamic resizing before the framebuffer stuff can go in, otherwise it takes up too much memory up-front.
    const color_rgb_s *palette = nullptr;
    resolution_s r = {0, 0, 0};
};

const frame_buffer_s* kr_acquire_renderer(void);

const frame_buffer_s* kr_framebuffer_ptr(void);

void kr_report_total_texture_size(void);

void kr_set_depth_testing_enabled(const bool enabled);

void kr_upload_texture(texture_c *const tex);

void kr_re_upload_texture(const texture_c *const tex);

void kr_release_renderer(void);

void kr_rasterize_mesh(const std::vector<triangle_s> &scene, const bool wireframe);

void kr_depth_sort_mesh(std::vector<triangle_s> *const mesh);

void kr_clear_frame(void);

#endif
