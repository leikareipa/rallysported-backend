/*
 * 20187, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef PALAT_H
#define PALAT_H

#include "types.h"

class texture_c;

void kpalat_initialize_palat(const file_handle_t fileHandle);

void kpalat_release_palat(void);

void kpalat_set_pala_pixel(const u8 palaIdx, const u8 x, const u8 y, const u8 color);

const texture_c *kpalat_pala_ptr(const uint idx, const bool filtered = true);

uint kpalat_num_palas(void);

uint kpalat_max_num_palas(void);

uint kpalat_pala_width(void);

uint kpalat_pala_height(void);

bool kpalat_save_palat(void);

#endif
