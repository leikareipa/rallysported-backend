/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 * An object for storing and accessing information related to a single texture.
 *
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstring>
#include "../core/display.h"
#include "../core/render.h"
#include "../core/common.h"
#include "../core/memory.h"

class texture_c
{
public:
    texture_c()
    {
        return;
    }
    texture_c(const resolution_s &resolution, const bool allocMem = true)
    {
        initialize(resolution, nullptr, allocMem);

        return;
    }

    void initialize(const resolution_s &resolution, const char *const reason = nullptr, const bool allocMem = true)
    {
        k_assert(pixels.is_null(), "Tried to initialize a texture whose pixel data was not null.");

        paddedRes = unpaddedRes = resolution;
        if (allocMem)
        {
            pixels.alloc((this->width() * this->height() * (this->bpp() / 8)),
                         (reason == nullptr? "Generic texture" : reason));
        }

        return;
    }

    // Upload to or otherwise make the texture available for use by the renderer.
    //
    void make_available_to_renderer()
    {
        if (!this->isRegistered)
        {
            kr_upload_texture(this);
            isRegistered = true;
        }
        else
        {
            kr_re_upload_texture(this);
        }

        return;
    }

    // Rally-Sport in many cases includes non-power-of-two textures, which don't
    // play nice with older 3d cards especially. In these cases, you can pad the
    // textures to a power-of-two.
    //
    void pad_to_size(uint newWidth, uint newHeight)
    {
        k_assert(!this->isPadded, "Was asked to re-pad a texture. Not allowed.");

        k_assert(!this->pixels.is_null(), "Was asked to pad a null texture. Can't comply.");
        k_assert(newWidth >= this->width(), "Was asked to pad to a smaller than real size. Not allowed.");
        k_assert(newHeight >= this->height(), "Was asked to pad to a smaller than real size. Not allowed.");

        k_assert(((newWidth <= MAX_TEXTURE_WIDTH) && (newHeight <= MAX_TEXTURE_HEIGHT)),
                 "Was asked to pad a texture to dimensions too large.");

        // Don't need to pad if we already have the right size.
        if ((newWidth == this->width()) &&
                (newHeight == this->height()))
        {
            return;
        }

        // Create a new texture that has the unpadded texture's data + padding.
        texture_c paddedTex({newWidth, newHeight, this->paddedRes.bpp});
        paddedTex.pixels.alloc(newWidth * newHeight, "Pixel buffer for texture-padding");
        for (uint y = 0; y < newHeight; y++)
        {
            if (y < this->height()) // Copy the original data vertically.
            {
                for (uint x = 0; x < this->width(); x++)
                {
                    paddedTex.pixels[x + y * newWidth] = this->pixels[x + y * this->width()];
                }
            }
            else    // Repeat the original data over empty padded regions vertically.
            {
                for (uint x = 0; x < this->width(); x++)
                {
                    paddedTex.pixels[x + y * newWidth] = this->pixels[x + (this->height() - 1) * this->width()];
                }
            }

            // At the horizontal tails of the pixel rows, repeat the original data over
            // empty padded regions.
            if (newWidth > this->width())
            {
                const uint startOffsNew = (this->width() + (y * newWidth));
                const uint startOffsOld = ((this->width() - 1) + (y < this->height()? y : ((this->height() - 1)) * this->width()));
                memset(&paddedTex.pixels[startOffsNew], this->pixels[startOffsOld],
                       paddedTex.pixels.up_to(startOffsNew + (newWidth - this->width())));
            }
        }

        // Replace the unpadded texture with the padded one.
        const resolution_s prevSize = this->unpaddedRes;
        this->pixels.release_memory();
        *this = paddedTex;
        this->unpaddedRes = prevSize;
        this->isPadded = true;

        return;
    }

    void pad_to_pot_bounding_rectangle(void)
    {
        this->pad_to_size(nearest_power_of_two(this->paddedRes.w, MAX_TEXTURE_WIDTH),
                          nearest_power_of_two(this->paddedRes.h, MAX_TEXTURE_WIDTH));

        return;
    }

    void pad_to_pot_bounding_square(void)
    {
        const uint sideLen = (nearest_power_of_two(this->paddedRes.w, MAX_TEXTURE_WIDTH) > nearest_power_of_two(this->paddedRes.h, MAX_TEXTURE_HEIGHT)?
                                  nearest_power_of_two(this->paddedRes.w, MAX_TEXTURE_WIDTH) : nearest_power_of_two(this->paddedRes.h, MAX_TEXTURE_HEIGHT));

        this->pad_to_size(sideLen, sideLen);

        return;
    }

    bool is_padded(void) const
    {
        return isPadded;
    }

    // The texture's width, including any padding.
    //
    uint width() const
    {
        return (isPadded? paddedRes.w : unpaddedRes.w);
    }

    uint height() const
    {
        return (isPadded? paddedRes.h : unpaddedRes.h);
    }

    uint bpp() const
    {
        return unpaddedRes.bpp;
    }

    // The original width of the texture, ignoring any padding.
    //
    uint width_unpadded() const
    {
        return unpaddedRes.w;
    }

    uint height_unpadded() const
    {
        return unpaddedRes.h;
    }

    heap_bytes_s<palette_idx_t> pixels;
    bool isFiltered = false;                // Whether to apply texture filtering to this texture when rendering.
    uint customParams[2];                   // For storing custom data, if/when needed.

private:
    resolution_s paddedRes = {0, 0, 0};     // Texture resolution, including any padding.
    resolution_s unpaddedRes = {0, 0, 0};   // Texture resolution, without padding.
    bool isRegistered = false;              // Whether this texture has been registered with the renderer.
    bool isPadded = false;                  // Set to true if this texture has been padded.

    // Returns the nearest (largest) power-of-two value for the given value.
    //
    uint nearest_power_of_two(const uint v, const uint maxV)
    {
        uint pot = 2;

        while (pot < v)
        {
            pot *= 2;

            k_assert(pot <= maxV, "Power-or-two value exceeds given maximum.");
        }

        k_assert(pot >= v, "Power-of-two value is less than initial value.");

        return pot;
    }
};

#endif
