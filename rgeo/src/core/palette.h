/*
 * 2017, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef PALETTE_H
#define PALETTE_H

#include "common.h"

void kpal_initialize_palettes();

void kpal_release_palette(void);

const color_rgb_s* kpal_current_palette_ptr(void);

color_rgb_s kpal_index_to_rgb(const uint idx);

uint kpal_num_shade_levels(void);

uint kpal_palette_size(void);

uint kpal_num_palettes(void);

uint kpal_num_primary_colors(void);

#endif
