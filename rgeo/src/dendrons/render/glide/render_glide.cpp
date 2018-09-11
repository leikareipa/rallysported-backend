/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED Glide 3.x renderer
 *
 */

#include <glide/glide.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

#include "../../../core/display.h"
#include "../../../core/display.h"
#include "../../../core/texture.h"
#include "../../../core/palette.h"
#include "../../../core/render.h"
#include "../../../core/common.h"
#include "../../../core/palat.h"
#include "../../../core/types.h"

static frame_buffer_s FRAME_BUFFER;

// How many bytes of texture data have been registered.
static uint TOTAL_TEXTURE_SIZE = 0;

// The address of the beginning of Glide's texture memory (assumed for TMU0).
static uint GLIDE_STARTING_TEXTURE_ADDRESS = 0;

// The address in texture memory (assumed for TMU0) when the next texture can be placed.
// This will be increased as more textures are loaded.
static uint CURRENT_GLIDE_TEXTURE_ADDRESS = 0;

// The vertex structure we want Glide to use. This matches the parameters and
// their order that we give via grVertexLayout() in the initializer function.
struct glide_vert_s
{
	float x, y;
	float w;
	float r, g, b;
	float s, t;
};

void kr_report_total_texture_size(void)
{
    DEBUG(("Size of texture data registered: %u KB.", (TOTAL_TEXTURE_SIZE / 1024)));

    return;
}

static void create_glide_fog_table(void)
{
	FxI32 fogTableSize;
	grGet(GR_FOG_TABLE_ENTRIES, 4, &fogTableSize);

	GrFog_t fog[fogTableSize];

	/// TODO. This doesn't generate very good fog in our case. Make it better.
	guFogGenerateLinear(fog, 3700, 4500);

	grFogTable(fog);

	return;
}

static void initialize_glide(void)
{
	// Vertex layout.
	grVertexLayout(GR_PARAM_XY, 0, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Q, 8, GR_PARAM_ENABLE); 
	grVertexLayout(GR_PARAM_RGB, 12, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_ST0, 24, GR_PARAM_ENABLE);

	grColorMask(FXTRUE, FXFALSE);

	// Fog.
	create_glide_fog_table();
	grFogMode(GR_FOG_WITH_TABLE_ON_Q);
	grFogColorValue(0);

	// Alpha testing.
	grAlphaTestFunction(GR_CMP_GREATER);
	grAlphaTestReferenceValue(0);

	// Depth testing.
	grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
	grDepthBufferFunction(GR_CMP_LESS); 
	grDepthMask(FXTRUE);

	// Texturing.
	grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
	grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
	grTexCombine(GR_TMU0, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE); // Configure for decal texturing.
	grTexMipMapMode(GR_TMU0, GR_MIPMAP_DISABLE, FXFALSE);

	CURRENT_GLIDE_TEXTURE_ADDRESS = grTexMinAddress(GR_TMU0);
	GLIDE_STARTING_TEXTURE_ADDRESS = CURRENT_GLIDE_TEXTURE_ADDRESS;

	return;
}

const frame_buffer_s* kr_acquire_renderer(void)
{
	FRAME_BUFFER.r = kd_display_resolution();
	FRAME_BUFFER.palette = kpal_current_palette_ptr();

	DEBUG(("Glide system report:"));
	DEBUG(("\tVendor: '%s'.", grGetString(GR_VENDOR)));
	DEBUG(("\tBoard: '%s'.", grGetString(GR_HARDWARE)));
	DEBUG(("\tTexture memory: %u KB", (grTexMaxAddress(GR_TMU0) - grTexMinAddress(GR_TMU0)) / 1024));
	DEBUG(("\tExtensions: '%s'.", grGetString(GR_EXTENSION)));
	DEBUG(("\tRenderer: '%s' (%s).", grGetString(GR_RENDERER), grGetString(GR_VERSION)));

	initialize_glide();

    return &FRAME_BUFFER;
}

void kr_depth_sort_mesh(std::vector<triangle_s> *const mesh)
{
	/// Not used.

	(void)mesh;

	return;
}

void kr_set_depth_testing_enabled(const bool enabled)
{
	if (enabled)
	{
		grDepthBufferFunction(GR_CMP_LESS); 
	}
	else
	{
		grDepthBufferFunction(GR_CMP_ALWAYS); 
	}

    return;
}

void kr_release_renderer(void)
{
	grGlideShutdown();

    return;
}

void kr_clear_frame(void)
{
	grBufferClear(0, 0, 65535);

	return;
}

static GrLOD_t glide_lod_for_size(const uint size)
{
	switch(size)
	{
		case 4: return GR_LOD_LOG2_4;
		case 8: return GR_LOD_LOG2_8;
		case 16: return GR_LOD_LOG2_16;
		case 32: return GR_LOD_LOG2_32;
		case 64: return GR_LOD_LOG2_64;
		case 128: return GR_LOD_LOG2_128;
		case 256: return GR_LOD_LOG2_256;

		default: k_assert(0, "Failed to find a suitable LOD for the given size.");
	}
}

static GrTexInfo generate_glide_tex_info_for_texture(const texture_c *const tex)
{
	GrTexInfo info;

	info.smallLodLog2 = glide_lod_for_size(tex->width());
	info.largeLodLog2 = glide_lod_for_size(tex->height());
	info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
	info.format = GR_TEXFMT_ARGB_1555;
	info.data = NULL;

	return info;
}

// Makes a local copy of the given texture's pixels and converts them to 16-bit color.
// Since this returns a pointer to the local static array, it's not thread-safe and
// the data is likely to change the next time this function is called.
// Assumes that the incoming texture is 8-bit.
//
FxU16* texture_pixels_as_16bit(const texture_c *const tex)
{
	static FxU16 cTex[MAX_TEXTURE_WIDTH * MAX_TEXTURE_HEIGHT];

	for (uint i = 0; i < (tex->width() * tex->height()); i++)
	{
	    const color_rgb_s c = FRAME_BUFFER.palette[tex->pixels[i]];

        cTex[i] =   (c.b / 8);
        cTex[i] |= ((c.g / 8) << 5);
        cTex[i] |= ((c.r / 8) << 10);
        cTex[i] |= ((tex->pixels[i] == 0? 0 : 1) << 15);
	}

	return cTex;
}

uint upload_texture(texture_c *const tex)
{
	uint texId = 0;
	FxU16 cTex[tex->width() * tex->height()];	// For if we need to color-convert the texture.
	GrTexInfo texInfo = generate_glide_tex_info_for_texture(tex);

	k_assert(!tex->pixels.is_null(),
			 "Was asked to upload a NULL texture. Can't do that.");

	// The expectation is that all textures come in as 8-bit.
	k_assert(tex->bpp() == 8, "Was asked to upload a non-8-bit texture. Not allowed.");

	// We want Glide to store all textures internally as 16-bit, so convert.
	texInfo.data = texture_pixels_as_16bit(tex);

	const FxU32 textureSize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &texInfo); 
	tex->customParams[0] = CURRENT_GLIDE_TEXTURE_ADDRESS;
	if ((tex->customParams[0] + textureSize) > grTexMaxAddress(GR_TMU0))
	{
		k_assert(0, "Glide ran out of texture memory. Can't continue.");
	}
	else
	{
		grTexDownloadMipMap(GR_TMU0, tex->customParams[0], GR_MIPMAPLEVELMASK_BOTH, &texInfo);
		CURRENT_GLIDE_TEXTURE_ADDRESS += textureSize;
	}

	return texId;
}

void kr_upload_texture(texture_c *const tex)
{
	k_assert(tex->width() == tex->height(), "Detected a non-square texture. They're not allowed.");

	upload_texture(tex);

	TOTAL_TEXTURE_SIZE += (tex->width() * tex->height() * 2);	// *2 since we upload textures as 16-bit.

	return;
}

void kr_re_upload_texture(const texture_c *const tex)
{
	GrTexInfo texInfo = generate_glide_tex_info_for_texture(tex);
	texInfo.data = texture_pixels_as_16bit(tex);

	grTexDownloadMipMap(GR_TMU0, tex->customParams[0], GR_MIPMAPLEVELMASK_BOTH, &texInfo);

	return;
}

void kr_rasterize_mesh(const std::vector<triangle_s> &scene,
					   const bool wireframe)
{
	for (const auto &t: scene)
	{
	
		const texture_c *const tex = t.texturePtr;
		color_rgb_s c = FRAME_BUFFER.palette[t.baseColor];

		glide_vert_s v[3];

		v[0].w = 1.0/t.v[0].w;
		v[1].w = 1.0/t.v[1].w;
		v[2].w = 1.0/t.v[2].w;
	
		if (t.texturePtr == NULL)
		{
			// Color combine for solid fill.
			grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
		}
		else
		{
			const uint glideTexScale = (256 / tex->width());	// All Glide s,t coordinates need to be scaled to the range 0..256. (Assumes rectangular textures.)
			GrTexInfo texInfo = generate_glide_tex_info_for_texture(tex);

			c = {255, 255, 255};

			v[0].s = (t.v[0].uv[0] / (real)t.v[0].w) * glideTexScale;
			v[0].t = (t.v[0].uv[1] / (real)t.v[0].w) * glideTexScale;
			v[1].s = (t.v[1].uv[0] / (real)t.v[1].w) * glideTexScale;
			v[1].t = (t.v[1].uv[1] / (real)t.v[1].w) * glideTexScale;
			v[2].s = (t.v[2].uv[0] / (real)t.v[2].w) * glideTexScale;
			v[2].t = (t.v[2].uv[1] / (real)t.v[2].w) * glideTexScale;

			if (tex->isFiltered)
			{
				grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
			}
			else
			{
				grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_POINT_SAMPLED, GR_TEXTUREFILTER_POINT_SAMPLED);
			}

			grTexSource(GR_TMU0, tex->customParams[0], GR_MIPMAPLEVELMASK_BOTH, &texInfo);

			// Color combine for texture fill.
			grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
		}

		if (t.baseColor == TEXTURE_ALPHA_ENABLED)
		{
			//grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ONE, GR_BLEND_ZERO);
			grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
		}
		else
		{
			// Disable alpha blending.
			//grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO);
			grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_ONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
		}

		v[0].r = v[1].r = v[2].r = c.r;
		v[0].g = v[1].g = v[2].g = c.g;
		v[0].b = v[1].b = v[2].b = c.b;

		v[0].x = t.v[0].x;
		v[0].y = t.v[0].y;
		v[1].x = t.v[1].x;
		v[1].y = t.v[1].y;
		v[2].x = t.v[2].x;
		v[2].y = t.v[2].y;

		grDrawTriangle(&v[0], &v[1], &v[2]);

		if (wireframe &&
			t.interact.type == INTERACTIBLE_GROUND)
		{
			grDepthBiasLevel(-1);

			grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FXFALSE);
			grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_ONE, GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FXFALSE);
			grConstantColorValue(0xff000000);	// Black with 255 alpha.

			grDrawLine(&v[0], &v[1]);
			grDrawLine(&v[1], &v[2]);
			grDrawLine(&v[2], &v[0]);

			grDepthBiasLevel(0);
		}
	}

	return;
}

void kr_draw_mouse_cursor(frame_buffer_s *const fb,
                          const uint x, const uint y)
{
    return;
}


