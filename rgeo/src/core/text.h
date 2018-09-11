/*
 * 2017 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef TEXT_H
#define TEXT_H

#include "types.h"

void ktext_initialize_text(void);

void ktext_release_text(void);

uint ktext_font_height(void);

uint ktext_font_width(void);

uint ktext_font_unpadded_width(void);

const texture_c* ktext_character_texture(const char c);

#endif
