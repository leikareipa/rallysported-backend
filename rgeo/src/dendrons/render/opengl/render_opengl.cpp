/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED OpenGL renderer
 *
 */

#include <gl/gl.h>
#include <gl/glext.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "../../../core/ui/interactible.h"
#include "../../../core/texture.h"
#include "../../../core/display.h"
#include "../../../core/palette.h"
#include "../../../core/render.h"
#include "../../../core/common.h"
#include "../../../core/types.h"

static frame_buffer_s FRAME_BUFFER;

// How many bytes of texture data have been registered.
static u32 TOTAL_SIZE_SIZE = 0;

const frame_buffer_s* kr_acquire_renderer(void)
{
	FRAME_BUFFER.r = kd_display_resolution();

    return &FRAME_BUFFER;
}

void kr_report_total_texture_size(void)
{
    DEBUG(("Size of texture data registered: %u KB.", (TOTAL_SIZE_SIZE / 1024)));

    return;
}

void kr_set_depth_testing_enabled(const bool enabled)
{
	enabled? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);

    return;
}

void kr_depth_sort_mesh(std::vector<triangle_s> *const mesh)
{
	/// Not used.

	(void)mesh;

	return;
}

void kr_release_renderer(void)
{
	/// Not used.

    return;
}

void kr_clear_frame(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return;
}

// Makes a local copy of the given texture's pixels and converts them to 32-bit color.
// Since this returns a pointer to the local static array, it's not thread-safe and
// the data is likely to change the next time this function is called.
//
const u8* texture_pixels_as_32bit(const texture_c *const tex)
{
	static u8 cTex[MAX_TEXTURE_WIDTH * MAX_TEXTURE_HEIGHT * 4];

	if (tex->bpp() == 32)
	{
		return tex->pixels.ptr();
	}

	memset(cTex, 0, tex->width() * tex->height() * 4);

	for (uint k = 0; k < (tex->width() * tex->height()); k++)
	{
		color_rgb_s c = kpal_index_to_rgb(tex->pixels[k]);

		cTex[k*4+0] = c.r;
		cTex[k*4+1] = c.g;
		cTex[k*4+2] = c.b;
		cTex[k*4+3] = tex->pixels[k] == 0? 0 : 255;  // Palette index 0 is transparent.
	}

	return cTex;
}

uint upload_texture(texture_c *const tex)
{
	uint texId;
	const u8 *texData = tex->pixels.ptr();
	uint scaleMode = tex->isFiltered? GL_LINEAR : GL_NEAREST;

	k_assert(!tex->pixels.is_null(),
			 "Was asked to upload a NULL texture. Can't do that.");

	// If the texture is paletted, convert it to 32-bit.
	if (tex->bpp() == 8)
	{
		texData = texture_pixels_as_32bit(tex);
	}
	else if (tex->bpp() != 32)
	{
		k_assert(0, "Was asked to upload a non-32-bit, non-8-bit texture. Can't do.");
	}

	glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scaleMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scaleMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->width(), tex->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);

	TOTAL_SIZE_SIZE += (tex->width() * tex->height() * 4);	// We register as 32-bit textures.

	return texId;
}

void kr_re_upload_texture(const texture_c *const tex)
{
	const uint texId = tex->customParams[0];
	const u8 *const texData = texture_pixels_as_32bit(tex);

	glBindTexture(GL_TEXTURE_2D, texId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->width(), tex->height(),
                    GL_RGBA, GL_UNSIGNED_BYTE, texData);

	return;
}

void kr_upload_texture(texture_c *const tex)
{
	const uint texId = upload_texture(tex);

	tex->customParams[0] = texId;

	return;
}

void kr_rasterize_mesh(const std::vector<triangle_s> &scene,
					   const bool wireframe)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (const auto &t: scene)
	{
		const texture_c *const tex = t.texturePtr;
		u8 alpha = 255;

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Toggle alpha on/off depending on whether the triangle is to have it.
		(t.baseColor == TEXTURE_ALPHA_ENABLED)? glEnable(GL_ALPHA_TEST) : glDisable(GL_ALPHA_TEST);

		// If the triangle has no texture, draw it with a solid fill of its base color.
		if (t.texturePtr == NULL)
		{
			const color_rgb_s c = kpal_index_to_rgb(t.baseColor);

			glDisable(GL_TEXTURE_2D);
			glColor4ub(c.r, c.g, c.b, alpha);

			glBegin(GL_TRIANGLES);
				glVertex3f(t.v[0].x, -t.v[0].y, -t.v[0].w);
				glVertex3f(t.v[1].x, -t.v[1].y, -t.v[1].w);
				glVertex3f(t.v[2].x, -t.v[2].y, -t.v[2].w);
			glEnd();
		}
		else // Otherwise, draw the triangle with a texture.
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, tex->customParams[0]);
			glColor4ub(255, 255, 255, alpha);

			glBegin(GL_TRIANGLES);
				glTexCoord2f(t.v[0].uv[0]/tex->width(), t.v[0].uv[1]/tex->height());
				glVertex3i(t.v[0].x, -t.v[0].y, -t.v[0].w);
				glTexCoord2f(t.v[1].uv[0]/tex->width(), t.v[1].uv[1]/tex->height());
				glVertex3i(t.v[1].x, -t.v[1].y, -t.v[1].w);
				glTexCoord2f(t.v[2].uv[0]/tex->width(), t.v[2].uv[1]/tex->height());
				glVertex3i(t.v[2].x, -t.v[2].y, -t.v[2].w);
			glEnd();
		}
	}

	if (wireframe)
	{
		glLineWidth(2.5);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glDisable(GL_TEXTURE_2D);
		glColor4ub(0, 0, 0, 255);

		glBegin(GL_TRIANGLES);
		for (uint i = 0; i < scene.size(); i++)
		{
			const triangle_s &t = scene[i];

			// Only show wireframe for ground tiles.
			if (t.interact.type != INTERACTIBLE_GROUND)
			{
				continue;
			}

			glVertex3f(t.v[0].x, -t.v[0].y, -t.v[0].w);
			glVertex3f(t.v[1].x, -t.v[1].y, -t.v[1].w);
			glVertex3f(t.v[2].x, -t.v[2].y, -t.v[2].w);
			
		}
		glEnd();
	}

	return;
}


