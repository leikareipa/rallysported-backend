/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef SOFTWARE_DRAW_H
#define SOFTWARE_DRAW_H

#include "../../../core/ui.h"

void rs_init_tri_filler(frame_buffer_s *const frameBuffer);

void rs_fill_tri(const triangle_s *const t);

#endif
