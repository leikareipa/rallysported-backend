/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED software renderer: texture manager (stub).
 *
 */

#include "../../../core/texture.h"
#include "../../../core/display.h"
#include "../../../core/types.h"
#include "../../../core/ui.h"

// How many bytes of texture data have been registered.
static u32 TOTAL_TEXTURE_SIZE = 0;

static void validate_texture_size(const texture_c *const tex)
{
    // Verify that the texture is power-of-two.
    {
        uint p = 2;
        while (p <= MAX_TEXTURE_WIDTH)
        {
            if (tex->width() == p)
            {
                goto width_good;
            }

            p *= 2;
        }
        k_assert(0, "Detected a non-power of two texture (or a texture exceeding allowable dimensions). Can't let it go about.");
        width_good:;

        p = 2;
        while (p <= MAX_TEXTURE_HEIGHT)
        {
            if (tex->height() == p)
            {
                goto height_good;
            }

            p *= 2;
        }
        k_assert(0, "Detected a non-power of two texture (or a texture exceeding allowable dimensions). Can't let it go about.");
        height_good:;
    }

    // For the benefit of simplicity in some renderers, e.g. Glide, always require
    // square textures.
    k_assert((tex->width() == tex->height()), "Detected a non-square texture. Only square textures, please.");

    return;
}

void kr_re_upload_texture(const texture_c *const tex)
{
    /// Note: The software renderer doesn't need to upload textures, so nothing
    /// of note needs to happen in this function.

    // For debugging reasons, validate the texture.
    validate_texture_size(tex);

    return;
}

void kr_report_total_texture_size(void)
{
    DEBUG(("Size of texture data registered: %u KB.", (TOTAL_TEXTURE_SIZE / 1024)));

    return;
}

// Give the renderer a chance to e.g. upload the texture data to video memory.
// Must be called for each texture after being initialized but before use.
//
void kr_upload_texture(texture_c *const tex)
{
    /// Note: The software renderer doesn't need to upload textures, so nothing
    /// of note needs to happen in this function.

    // For debugging reasons, validate the texture.
    validate_texture_size(tex);

    TOTAL_TEXTURE_SIZE += (tex->width() * tex->height());

    return;
}

