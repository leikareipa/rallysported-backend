/*
 * 2017 Tarpeeksi Hyvae Soft /
 * RallySportED font storage
 *
 * Manages the program's font.
 *
 */

#include <cstdlib>
#include <cstring>
#include <vector>
#include "../../core/texture.h"
#include "../../core/display.h"
#include "../../core/ui.h"

#include "font.inc"

static std::vector<texture_c> CHARACTERS;

uint ktext_font_height(void)
{
    return FONT_HEIGHT;
}

// Returns the effective width of the font, i.e. without horizontal padding.
//
uint ktext_font_unpadded_width(void)
{
    return 5;
}

uint ktext_font_width(void)
{
    return FONT_WIDTH;
}

const texture_c* ktext_character_texture(const char c)
{
    const u8 idx = c  - ' ';

    k_assert(idx < CHARACTERS.size(),
             "Was asked for the texture of a font character out of bounds.");

    return &CHARACTERS[idx];
}

void ktext_initialize_text(void)
{
    const uint numChars = NUM_ELEMENTS(FONT) / (FONT_WIDTH * FONT_HEIGHT);

    for (uint i = 0; i < numChars; i++)
    {
        texture_c t({FONT_WIDTH, FONT_HEIGHT, 8}, "Font character texture");
        t.isFiltered = false;

        const uint idx = i * (FONT_WIDTH * FONT_HEIGHT);
        memcpy(t.pixels.ptr(), &FONT[idx], t.pixels.up_to(FONT_WIDTH * FONT_HEIGHT));
        t.make_available_to_renderer();

        CHARACTERS.push_back(t);
    }

    return;
}

void ktext_release_text(void)
{
    for (uint i = 0; i < CHARACTERS.size(); i++)
    {
        CHARACTERS[i].pixels.release_memory();
    }

    return;
}

