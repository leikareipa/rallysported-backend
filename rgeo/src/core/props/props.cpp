/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED track props
 *
 * Handles track props, i.e. the various 3d trackside objects like trees and signs.
 *
 */

#include <unordered_map>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include "../../core/geometry/geometry_sw.h"
#include "../../core/geometry.h"
#include "../../core/game_exe.h"
#include "../../core/texture.h"
#include "../../core/display.h"
#include "../../core/palette.h"
#include "../../core/camera.h"
#include "../../core/maasto.h"
#include "../../core/props.h"
#include "../../core/common.h"
#include "../../core/palat.h"
#include "../../core/file.h"

/*
 * TODOS:
 *
 * - (none at the moment.)
 *
 */

// A list of the types and triangle meshes of all the known props.
static const std::vector<std::pair<prop_id_e, const std::vector<triangle_s>*>> *PROPS;

// The game stores individual prop textures in one big texture file; this array
// will hold all of that file's pixels. Individual sub-textures will point to
// it.
static heap_bytes_s<u8> MASTER_TEXTURE_DATA;
static std::vector<texture_c> PROP_TEXTURES;    // Individual prop textures.

// For extracting the two 16-bit exe offset pointers from a prop's type id, which
// is a bit combination of these two. Assumes propId to be a u32 variable.
#define BYTE_OFFS_COORDS(propId) (((propId) >> 16) & 0x0000ffff)
#define BYTE_OFFS_INDICES(propId) ((propId) & 0x0000ffff)

// Returns the type of a random prop (currently used by the UI to cycle through all
// props).
/// TODO. Right now, just returns consecutive ones.
prop_id_e kprop_random_prop(void)
{
    static uint idx = 0;

    do
    {
        idx++;
        if (idx >= PROPS->size())
        {
            idx = 0;
        }
    } while ((*PROPS)[idx].first == PROP_STARTING_LINE ||       // Skip the starting lines - they're immutable at the moment.
             (*PROPS)[idx].first == PROP_STONE_STARTING_LINE);

    return (*PROPS)[idx].first;
}

// Returns the prop index corresponding with the given prop type. The index value
// is used in the manifesto file to identify the various props. Note that this
// function returns the value as 1-based, where the first prop has an index of 1,
// and so the return value isn't suitable for indexing into the PROPS list directly.
//
uint kprop_prop_idx_for_type(const prop_id_e propType)
{
    for (uint i = 0; i < PROPS->size(); i++)
    {
        if ((*PROPS)[i].first == propType)
        {
            return (i + 1);
        }
    }

    k_assert(0, "Failed to return a valid prop index.");
    return 0;
}

// Returns the given prop's type value as a 4-byte u8 string, for writing
// directly into Rally-Sport's binaries. Note that this takes the index of
// the prop as 1-based, where an index of 1 is the first prop.
//
void kprop_prop_bytestring_for_idx(u8 *const str, const uint idx)
{
    const auto prop = PROPS->at(idx - 1);
    const u32 typeVal = (u32)prop.first;
    const u8 typeStr[4] = {u8((typeVal >> 16) & 0x000000ff),
                           u8((typeVal >> 24) & 0x000000ff),
                           u8((typeVal >>  0) & 0x000000ff),
                           u8((typeVal >>  8) & 0x000000ff)};

    memcpy(str, typeStr, NUM_ELEMENTS(typeStr));

    return;
}

static const std::vector<triangle_s>* prop_mesh_for_type(const prop_id_e propType)
{
    for (const auto &prop: (*PROPS))
    {
        if (prop.first == propType)
        {
            return prop.second;
        }
    }

    k_assert(0, "Failed to return a valid prop mesh.");
    return nullptr;
}

static void load_prop_textures(void)
{
    const u32 expectedFileSize = 32768; /// For now, hard-code the expected size of TEXT1.DTA.
    const u32 texOffs = kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_PROP_TEXTURE_UVS);
    const file_handle_t fht = kfile_open_file("TEXT1.DTA", "rb");   // The object texture file.
    const file_handle_t fhe = kge_file_handle_rallye_exe();

    k_assert(kfile_file_size(fht) == expectedFileSize, "The master prop texture (TEXT1.DTA) has an unexpected file size. Can't continue.");

    // Obtain the texture data.
    MASTER_TEXTURE_DATA.alloc(expectedFileSize, "Master prop texture");
    kfile_read_byte_array(MASTER_TEXTURE_DATA.ptr(), MASTER_TEXTURE_DATA.up_to(expectedFileSize), fht);

    // Obtain texture uv coordinates, so we can split the main prop texture into
    // the smaller textures that are actually used on the props.
    /// TODO. Needs cleanup.
    kfile_seek(texOffs, fhe);
    while (1)
    {
        texture_c tex;
        uint texWidth, texHeight;
        u8 xOffs, yOffs;        // X,y, offset in the main texture of this subtexture.

        if (kfile_peek_value<u16>(fhe) == 0xffff)   // 0xffff marks stoppage.
        {
            break;
        }

        // Get the texture size.
        texWidth = kfile_read_value<u16>(fhe) / 2;
        texHeight = kfile_read_value<u16>(fhe) / 2;
        tex.initialize({texWidth, texHeight, 8}, "Prop texture");

        kfile_jump(2, fhe);

        xOffs = kfile_read_value<u8>(fhe);
        yOffs = kfile_read_value<u8>(fhe);
        yOffs *= 2;

        kfile_jump(2, fhe);

        tex.isFiltered = false;
        tex.pixels.alloc(tex.width() * tex.height(), "Prop sub-texture pixels");

        // Read in the texture data.
        for (uint y = 0; y < tex.height_unpadded(); y++)
        {
            for (uint x = 0; x < tex.width_unpadded(); x++)
            {
                tex.pixels[x + y * tex.width_unpadded()] = MASTER_TEXTURE_DATA[(xOffs + x) + (yOffs + y) * 128];
            }
        }

        // Make sure all of these textures are power-of-two. For MSI, we fit into a
        // rectangle to conserve video memory; otherwise we do square to serve the
        // 3dfx Voodoo, primarily.
        #ifdef RENDERER_IS_MSI
            tex.pad_to_fitting_rectangle();
        #else
            tex.pad_to_pot_bounding_square();
        #endif

       // INFO(("Texture size: %d x %d.", tex.width(), tex.height()));

        tex.make_available_to_renderer();

        PROP_TEXTURES.push_back(tex);
    }

    return;
}

static std::vector<triangle_s> load_prop_mesh(prop_id_e type, const file_handle_t fh)
{
    const u32 coordOffs = BYTE_OFFS_COORDS(type) + kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_DATA_SEGMENT);
    const u32 indexOffs = BYTE_OFFS_INDICES(type) + kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_DATA_SEGMENT);

    // Load the prop's vertex coordinates.
    std::vector<vector3<i32>> coords;
    {
        kfile_seek(coordOffs, fh);

        const u16 numCoords = kfile_read_value<u16>(fh);
        coords.reserve(numCoords);
        for (uint i = 0; i < numCoords; i++)
        {
            vector3<i32> v;

            v.x = kfile_read_value<i16>(fh);
            v.y = kfile_read_value<i16>(fh);
            v.z = -kfile_read_value<i16>(fh);

            coords.push_back(v);
        }
    }

    // Create the prop's triangle mesh.
    std::vector<triangle_s> triMesh;
    {
        kfile_seek(indexOffs, fh);

        // Create a triangle mesh of the prop'a polygons. This loops until it's parsed
        // the last polygon.
        /// TODO. Needs cleanup.
        while (1)
        {
            // Find which of the subtextures this polygon uses.
            u8 texIdx = kfile_read_value<u8>(fh);
            kfile_jump(1, fh);

            bool hasTexture = true;
            palette_idx_t c = 4;

            // Decide whether the polygon has a texture of whether it is filled with solid color.
            {
                if (texIdx < kpal_num_primary_colors())     // Use a solid color.
                {
                    hasTexture = false;
                    c = texIdx;
                }
                else                                    // Use a texture.
                {
                    texIdx -= kpal_num_primary_colors();
                }

                if (texIdx > 127)                       // Use a texture, but wrap index number around.
                {
                    texIdx -= 128;
                }
            }

            // Skip some bytes whose purpose we don't know.
            kfile_jump(8, fh);

            u8 numIndices = kfile_read_value<u8>(fh);
            kfile_jump(1, fh);

            // Fetch the vertex indices of this polygon.
            std::vector<uint> indices;
            {
                // First index.
                indices.push_back(kfile_read_value<u16>(fh));

                // Get the rest of the indices.
                for (int i = 0; i < (numIndices - 1); i++)
                {
                    const int idx = kfile_read_value<i16>(fh);
                    indices.push_back(idx);

                    k_assert(idx >= 0,
                             "Malformed index detected in a 3d model read from the game executable. Aborting.");

                    kfile_jump(2, fh);
                }
            }

            kfile_jump(2, fh);

            // Create a triangle mesh of the polygon by matching each vertex index
            // of each polygon to the corresponding entry in the list of vertex
            // coordinates.
            triangle_s t;

            t.interact.type = INTERACTIBLE_PROP;

            t.v[0].w = 1;
            t.v[1].w = 1;
            t.v[2].w = 1;

            t.v[0].uv[0] = 0;
            t.v[0].uv[1] = 0;
            t.v[1].uv[0] = 0;
            t.v[1].uv[1] = 0;
            t.v[2].uv[0] = 0;
            t.v[2].uv[1] = 0;

            t.baseColor = (hasTexture? TEXTURE_ALPHA_ENABLED : c);
            t.texturePtr = (hasTexture? &PROP_TEXTURES[texIdx] : NULL);

            uint firstMeshIdx = triMesh.size();

            for (uint i = 1; i < (indices.size() - 1); i++)
            {
                t.v[0].x = coords[indices[0]].x;
                t.v[0].y = coords[indices[0]].y;
                t.v[0].z = coords[indices[0]].z;

                t.v[1].x = coords[indices[i + 0]].x;
                t.v[1].y = coords[indices[i + 0]].y;
                t.v[1].z = coords[indices[i + 0]].z;

                t.v[2].x = coords[indices[i + 1]].x;
                t.v[2].y = coords[indices[i + 1]].y;
                t.v[2].z = coords[indices[i + 1]].z;

                triMesh.push_back(t);
            }

            /// Temporary. Assign u,v coordinates by hand.
            if (triMesh[firstMeshIdx].texturePtr != NULL)
            {
                if (indices.size() == 4)
                {
                    triangle_s *t = &triMesh[firstMeshIdx + 1];
                    const texture_c *tex = t->texturePtr;
                    t->v[0].uv[0] = 0;
                    t->v[0].uv[1] = 0;
                    t->v[1].uv[0] = tex->width_unpadded() - 0.1;
                    t->v[1].uv[1] = tex->height_unpadded() - 0.1;
                    t->v[2].uv[0] = 0;
                    t->v[2].uv[1] = tex->height_unpadded() - 0.1;

                    t = &triMesh[firstMeshIdx];
                    tex = t->texturePtr;
                    t->v[0].uv[0] = 0;
                    t->v[0].uv[1] = 0;
                    t->v[1].uv[0] = tex->width_unpadded() - 0.1;
                    t->v[1].uv[1] = 0;
                    t->v[2].uv[0] = tex->width_unpadded() - 0.1;
                    t->v[2].uv[1] = tex->height_unpadded() - 0.1;

                    /// Some props' 4-sided polys have flipped uvs. Need to unflip
                    /// them.
                    if (type == PROP_TRAFFIC_SIGN_EXCLAMATION ||
                        type == PROP_STONE_STARTING_LINE)
                    {
                        triangle_s *t = &triMesh[firstMeshIdx + 1];
                        const texture_c *tex = t->texturePtr;
                        t->v[0].uv[0] = 0;
                        t->v[0].uv[1] = tex->height_unpadded() - 0.1;
                        t->v[1].uv[0] = tex->width_unpadded() - 0.1;
                        t->v[1].uv[1] = 0;
                        t->v[2].uv[0] = 0;
                        t->v[2].uv[1] = 0;

                        t = &triMesh[firstMeshIdx];
                        tex = t->texturePtr;
                        t->v[0].uv[0] = 0;
                        t->v[0].uv[1] = tex->height_unpadded() - 0.1;
                        t->v[1].uv[0] = tex->width_unpadded() - 0.1;
                        t->v[1].uv[1] = tex->height_unpadded() - 0.1;
                        t->v[2].uv[0] = tex->width_unpadded() - 0.1;
                        t->v[2].uv[1] = 0;
                    }
                }
                else if (indices.size() == 5)
                {
                    triangle_s *t = &triMesh[firstMeshIdx + 2];
                    const texture_c *tex = t->texturePtr;
                    t->v[0].uv[0] = 0;
                    t->v[0].uv[1] = 0;
                    t->v[1].uv[0] = tex->width_unpadded() - 0.1;
                    t->v[1].uv[1] = tex->height_unpadded() - 0.1;
                    t->v[2].uv[0] = 0;
                    t->v[2].uv[1] = tex->height_unpadded() - 0.1;

                    t = &triMesh[firstMeshIdx + 1];
                    tex = t->texturePtr;
                    t->v[0].uv[0] = 0;
                    t->v[0].uv[1] = 0;
                    t->v[1].uv[0] = tex->width_unpadded() - 0.1;
                    t->v[1].uv[1] = 0;
                    t->v[2].uv[0] = tex->width_unpadded() - 0.1;
                    t->v[2].uv[1] = tex->height_unpadded() - 0.1;

                    t = &triMesh[firstMeshIdx];
                    tex = t->texturePtr;
                    t->v[0].uv[0] = 0;
                    t->v[0].uv[1] = 0;
                    t->v[1].uv[0] = tex->width_unpadded() - 0.1;
                    t->v[1].uv[1] = 0;
                    t->v[2].uv[0] = tex->width_unpadded() - 0.1;
                    t->v[2].uv[1] = 0;
                }
                else if (indices.size() == 3)
                {
                    triangle_s *t = &triMesh[firstMeshIdx];
                    const texture_c *tex = t->texturePtr;
                    t->v[0].uv[0] = 0;
                    t->v[0].uv[1] = tex->height_unpadded() - 0.1;
                    t->v[1].uv[0] = tex->width_unpadded() - 0.1;
                    t->v[1].uv[1] = tex->height_unpadded() - 0.1;
                    t->v[2].uv[0] = 0;
                    t->v[2].uv[1] = 0;
                }
                else    /// There are some polygons that don't have 3-5 points.
                {
                    /// TODO. Deal with these.
                  //  k_assert(0, "Unexpected polygon topology.");
                }
            }

            if (kfile_peek_value<u16>(fh) == 0xffff)    // 0xffff marks the end of the polygon entries.
            {
                break;
            }
        }
    }

    // Pre-depth-sort the triangles, so we might not need to do depth testing in rendering.
    std::sort(triMesh.begin(), triMesh.end(), [](const triangle_s &t1, const triangle_s &t2)
                                              {
                                                  const i32 d1 = (t1.v[0].z + t1.v[1].z + t1.v[2].z);
                                                  const i32 d2 = (t2.v[0].z + t2.v[1].z + t2.v[2].z);
                                                  return d1 < d2;
                                              });

    return triMesh;
}

// Adds the given prop's triangle mesh to the given destination mesh.
//
void kprop_add_prop_mesh(const track_prop_s &prop,
                         std::vector<triangle_s> *const dstMesh,
                         const int terrainHeight)
{
    const std::vector<triangle_s> *const propMesh = prop_mesh_for_type((prop_id_e)prop.propId);
    const uint firstTri = dstMesh->size();
    const uint lastTri = firstTri + propMesh->size();

    for (uint i = 0; i < propMesh->size(); i++)
    {
        triangle_s t = (*propMesh)[i];

        t.interact.params[INTERACT_PARAM_PROP_FIRST_TRI] = firstTri;
        t.interact.params[INTERACT_PARAM_PROP_LAST_TRI] = lastTri;
        t.interact.params[INTERACT_PARAM_PROP_IDX] = prop.idx;

        t.v[0].x += prop.pos.x + kcamera_initial_position()->x; /// Temp hack. The initial camera position needs to be nullified. Look into this later.
        t.v[0].y += prop.pos.y + terrainHeight;
        t.v[0].z += prop.pos.z - kcamera_initial_position()->z;

        t.v[1].x += prop.pos.x + kcamera_initial_position()->x;
        t.v[1].y += prop.pos.y + terrainHeight;
        t.v[1].z += prop.pos.z - kcamera_initial_position()->z;

        t.v[2].x += prop.pos.x + kcamera_initial_position()->x;
        t.v[2].y += prop.pos.y + terrainHeight;
        t.v[2].z += prop.pos.z - kcamera_initial_position()->z;

        dstMesh->push_back(t);
    }

    return;
}

// Loads the game's 3d track object meshes from RALLYE.EXE. Assumes that this is
// a valid Rally-Sport demo executable.
//
void kprop_initialize_props(void)
{
    load_prop_textures();

    // Load all known hardcoded prop meshes. Note that these have to be loaded in
    // the order dictated by the RallySportED loader (look at 'object_header' in
    // main.asm, there).
    {
        const file_handle_t fh = kge_file_handle_rallye_exe();

        #define prop(propId) { propId, [=]{ static const auto v = load_prop_mesh(propId, fh); return &v; }() }

        static const std::vector<std::pair<prop_id_e, const std::vector<triangle_s>*>> propList =
                    {prop(PROP_TREE),
                     prop(PROP_WIRE_FENCE),
                     prop(PROP_HORSE_FENCE),
                     prop(PROP_TRAFFIC_SIGN_80),
                     prop(PROP_TRAFFIC_SIGN_EXCLAMATION),
                     prop(PROP_STONE_ARCH),
                     prop(PROP_STONE_POST),
                     prop(PROP_LARGE_ROCK),
                     prop(PROP_SMALL_ROCK),
                     prop(PROP_LARGE_BILLBOARD),
                     prop(PROP_SMALL_BILLBOARD),
                     prop(PROP_BUILDING),
                     prop(PROP_UTIL_POLE_1),
                     prop(PROP_UTIL_POLE_2),
                     prop(PROP_STARTING_LINE),
                     prop(PROP_STONE_STARTING_LINE)};

        PROPS = &propList;

        #undef prop
    }

    return;
}

void kprop_release_props(void)
{
    MASTER_TEXTURE_DATA.release_memory();

    for (uint i = 0; i < PROP_TEXTURES.size(); i++)
    {
        PROP_TEXTURES[i].pixels.release_memory();
    }

    return;
}
