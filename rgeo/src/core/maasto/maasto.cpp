/*
 * 2017, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED track data
 *
 * Handles data stored in Rally-Sport's track files; namely, the MAASTO (heightmap)
 * and VARIMAA (tilemap) ones.
 *
 * Call the initializer function, after which you can call the various other ones
 * to access the track data.
 *
 */

#include <vector>
#include <list>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "../../core/manifesto.h"
#include "../../core/game_exe.h"
#include "../../core/geometry.h"
#include "../../core/fixedpt.h"
#include "../../core/display.h"
#include "../../core/texture.h"
#include "../../core/memory.h"
#include "../../core/camera.h"
#include "../../core/maasto.h"
#include "../../core/common.h"
#include "../../core/props.h"
#include "../../core/palat.h"
#include "../../core/file.h"
#include "../../core/ui.h"

/*
 * TODOS:
 *
 * - shadows for the ground. visually, they need to match rally-sport and ideally be
 *   calculable in real-time (while the heightmap is being edited) on sub-100 mhz pentiums.
 *
 */

// Variables that allow recall of where the track's data is located in the project
// container file. Useful when we want to save the data back to disk, etc.
static file_handle_t SOURCE_FILE_HANDLE = 0;        // A handle to the file containing the MAASTO and VARIMAA data.
static uint MAASTO_FILESIZE = 0;                    // Number of bytes that make up the MAASTO data.
static uint VARIMAA_FILESIZE = 0;
static uint MAASTO_FILEOFFS = 0;                    // Start of the MAASTO element in the container file.
static uint VARIMAA_FILEOFFS = 0;

// A list of the 3d objects on this track.
static std::vector<track_prop_s> TRACK_PROPS_POS;

// The height of each tile on the track.
static heap_bytes_s<int> TRACK_HEIGHTMAP;

// The PALA texture idx of each tile on the track.
static heap_bytes_s<u8> TRACK_TILEMAP;

// The coordinates of the currently-highlighted (e.g. by mouse hover) tile.
static vector2<int> HIGHLIGHTED_TILE_COORDS = {0, 0};

// Each track has one checkpoint near which the car must pass to have the lap be
// counted as valid. This holds the tile x,y coordinates of that checkpoint for
// the current track.
static vector2<int> TRACK_CHECKPOINT = {0, 0};

// The heightmap's extrema, top and bottom.
static const int HEIGHTMAP_VALUE_MAX = 255;
static const int HEIGHTMAP_VALUE_MIN = -510;

// Dimensions in world-units of each ground tile. Change these and the track
// will be drawn larger or smaller; although you'll need to adjust a number
// of other variables as well to get correct render output.
static const uint WIDTH_OF_TILE = 128;
static const uint HEIGHT_OF_TILE = 128;

// The height at which bodies of water are level on the current track.
static int WATER_LEVEL = 0;
static texture_c WATER_LEVEL_TEX;   // A non-canon texture for the editor to indicate water level.

// The track's tilemap converted into a smaller topdown image for UI display.
static texture_c TOPDOWN_MAP;

// The track's dimensions, in tile coordinates.
static u8 TRACK_WIDTH = 0;
static u8 TRACK_HEIGHT = 0;

// Converts the game's 2-byte height code into a proper signed value.
//
int kmaasto_game_height_to_editor_height(const u8 b1, const u8 b2)
{
    int height = 0;

    if (b2 == 1)            // Special case: more than -255 below ground level.
    {
        height = -256 - b1;
    }
    else                    // Above ground when b2 == 255, otherwise below ground.
    {
        height = b2 - b1;
    }

    return height;
}

// Converts the editor's signed height value into the game's 2-byte
// format.
//
std::pair<u8, u8> kmaasto_editor_height_to_game_height(const int h)
{
    u8 b1 = 0, b2 = 0;

    if (h > 0)              // Above ground.
    {
        b2 = 255;
        b1 = 255 - h;
    }
    else if (h <= 0)        // Below ground, or at ground level if h == 0.
    {
        if (h < -255)       // Special case of more than -255 below ground.
        {
            b2 = 1;
            b1 = abs(h) - 256;
        }
        else
        {
            b2 = 0;
            b1 = abs(h);
        }
    }
    else
    {
        k_assert(0, "Unhandled case when converting heights.");
    }

    return {b1, b2};
}

// Convert our heightmap back into Rally-Sport's two-byte format for exporting
// it to disk.
//
static void convert_maasto_for_export(u8 *const dst, const uint len)
{
    k_assert((len <= (kmaasto_track_width() * kmaasto_track_height())),
             "Was asked to convert the MAASTO for export, but the data length given was mismatched.");

    for (uint i = 0; i < len; i++)
    {
        const int h = TRACK_HEIGHTMAP[i];
        const auto heightWord = kmaasto_editor_height_to_game_height(h);

        dst[i*2] = heightWord.first;
        dst[i*2+1] = heightWord.second;
    }

    return;
}

// The game by default has four different 'skins' for spectators, and it decides
// which skin a spectator will be given based on the spectator's x,y coordinates
// on the track. This will return the correct skin for the given coordinates.
//
static u8 spectator_texture_at(const u8 tileX, const u8 tileY)
{
    const uint firstSpectatorTexIdx = 236;      // Index of the first PALA representing a (standing) spectator.
    const uint lastSpectatorTexIdx = 239;       // Index of the last PALA representing a (standing) spectator. Assumes consecutive arrangement.
    const uint numSkins = 4;
    const uint sameRows = 16;                   // The game will repeat the same pattern of variants on the x axis this many times.

    const uint yOffs = (tileY / sameRows) % numSkins;
    const uint texOffs = ((tileX + (numSkins - 1)) + (yOffs * (numSkins - 1))) % numSkins;

    const u8 textureIdx = (firstSpectatorTexIdx + texOffs);

    k_assert(((textureIdx >= firstSpectatorTexIdx) &&
              (textureIdx <= lastSpectatorTexIdx)),
             "Was going to return a spectator texture out of bounds. Not good.");

    return textureIdx;
}

bool kmaasto_save_track(void)
{
    heap_bytes_s<u8> convertedHeightmap(kmaasto_track_width() * kmaasto_track_height() * 2);

    DEBUG(("Saving the MAASTO data..."));

    // Save the MAASTO heightmap.
    convert_maasto_for_export(convertedHeightmap.ptr(), (kmaasto_track_width() * kmaasto_track_height()));
    kfile_seek(MAASTO_FILEOFFS, SOURCE_FILE_HANDLE);
    kfile_write_byte_array(convertedHeightmap.ptr(), convertedHeightmap.up_to(MAASTO_FILESIZE), SOURCE_FILE_HANDLE);

    // Save the VARIMAA tilemap.
    kfile_seek(VARIMAA_FILEOFFS, SOURCE_FILE_HANDLE);
    kfile_write_byte_array(TRACK_TILEMAP.ptr(), TRACK_TILEMAP.up_to(VARIMAA_FILESIZE), SOURCE_FILE_HANDLE);

    // Save the 3d objects' parameters into the manifesto.
    u8 params[5] = {(u8)TRACK_PROPS_POS.size()};
    kmanif_add_command_to_manifesto(MANIFCMD_NUM_PROPS, params);
    for (uint i = 0; i < TRACK_PROPS_POS.size(); i++)
    {
        auto &prop = TRACK_PROPS_POS[i];

        const u8 globalX = floor(prop.pos.x / (real)kmaasto_track_tile_width() / 2.0);
        const u8 globalY = floor(prop.pos.z / (real)kmaasto_track_tile_width() / 2.0);
        const u8 localX = ((prop.pos.x / (real)kmaasto_track_tile_width() / 2.0) - globalX) * 256;
        const u8 localY = ((prop.pos.z / (real)kmaasto_track_tile_width() / 2.0) - globalY) * 256;

        // It's not allowed to move the starting line, so skip those.
        if (prop.propId != PROP_STONE_STARTING_LINE &&
            prop.propId != PROP_STARTING_LINE)
        {
            // Save the position.
            params[0] = i + 1;
            params[1] = globalX;
            params[2] = globalY;
            params[3] = localX;
            params[4] = localY;
            kmanif_add_command_to_manifesto(MANIFCMD_MOVE_PROP, params);

            // Save the type.
            params[0] = i + 1;
            params[1] = kprop_prop_idx_for_type((prop_id_e)prop.propId);
            kmanif_add_command_to_manifesto(MANIFCMD_SET_PROP, params);
        }
    }
    kmanif_save_manifesto_file();

    kfile_flush_file(SOURCE_FILE_HANDLE);

    convertedHeightmap.release_memory();

    return true;
}

// Returns the texture index of the ground tile at the given coordinates.
//
static u8 texture_idx_at(const uint x, const uint y)
{
    if (x >= TRACK_WIDTH ||
        y >= TRACK_WIDTH)
    {
        return 0;
    }

    return TRACK_TILEMAP[x + y * TRACK_WIDTH];
}

// Creates a small pixelmap of the track's tilemap, to be used on the GUI.
//
static void generate_topdown_texture(void)
{
    const uint baseSize = 128 / kmaasto_track_width();
    const unsigned char skip = (kmaasto_track_tile_width() / TOPDOWN_MAP.width()) / baseSize;

    // Draw the minimap.
    for (uint y = 0; y < TOPDOWN_MAP.width(); y++)
    {
        for (uint x = 0; x < TOPDOWN_MAP.height(); x++)
        {
            const texture_c *const p = kpalat_pala_ptr(texture_idx_at((x * skip), (y * skip)));

            TOPDOWN_MAP.pixels[x + y * TOPDOWN_MAP.width()] = p->pixels[1];

            // Draw the checkpoint's location as a dot.
            if (x == (uint)TRACK_CHECKPOINT.x &&
                y == (uint)TRACK_CHECKPOINT.y)
            {
                TOPDOWN_MAP.pixels[x + y * TOPDOWN_MAP.width()] = 5;
            }
        }
    }


    return;
}

u8 kmaasto_texture_at(const uint x, const uint y)
{
    if (x >= TRACK_WIDTH ||
        y >= TRACK_WIDTH)
    {
        return 0;
    }

    return TRACK_TILEMAP[x + y * TRACK_WIDTH];
}

int kmaasto_height_at(const uint x, const uint y)
{
    if (x >= TRACK_WIDTH ||
        y >= TRACK_WIDTH)
    {
        return 0;
    }

    return TRACK_HEIGHTMAP[x + y * TRACK_WIDTH];// / 3.5;
}

// Parses the track's header in the game executable to get a list of the 3d props
// on this track.
//
static void load_track_prop_positions(const u8 trackId)
{
    u16 numObjs = 0;
    const file_handle_t fh = kge_file_handle_rallye_exe();

    kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_TRACK_PROP_BLOCK, trackId), fh);
    numObjs = kfile_read_value<u16>(fh);

    for (uint i = 0; i < numObjs; i++)
    {
        track_prop_s prop;
        u8 tileX, tileY;            // The prop's tile x,y position (divided by 2 in the game data).
        u8 subX, subY, height;      // Sub-tile position, i.e. a finer positioning within the coarser x,y tile position.
        u16 coordOffs, indexOffs;   // Exe offset pointers to the prop's vertex data.

        prop.isHighlighted = false;

        coordOffs = kfile_read_value<u16>(fh);
        indexOffs = kfile_read_value<u16>(fh);

        kfile_jump(2, fh);
        subX = kfile_read_value<u8>(fh);
        tileX = kfile_read_value<u8>(fh);
        subY = kfile_read_value<u8>(fh);
        tileY = kfile_read_value<u8>(fh);
        height = kfile_read_value<u8>(fh);

        prop.pos.x = (tileX * 2) * kmaasto_track_tile_width() + subX;
        prop.pos.y = (height == 255? 0 : (height - (kmaasto_track_tile_height() / 2)));
        prop.pos.z = (tileY * 2) * kmaasto_track_tile_height() + subY;

        static_assert(std::is_same<u32, decltype(prop.propId)>::value, "Expected u32.");
        static_assert(std::is_same<u16, decltype(coordOffs)>::value && std::is_same<u16, decltype(indexOffs)>::value, "Expected both to be u16.");
        prop.propId = (coordOffs << 16) | indexOffs;    // We can uniquely identify the game's props with a bit combination of these two offsets.

        prop.idx = TRACK_PROPS_POS.size();

        kfile_jump(1, fh);

        TRACK_PROPS_POS.push_back(prop);
    }

    return;
}

void kmaasto_change_prop_type(const uint propIdx)
{
    k_assert(propIdx < TRACK_PROPS_POS.size(), "Tried to cycle the type of an object whose index was out of bounds. No!!");

    // Don't allow changing the type of the starting position.
    if (TRACK_PROPS_POS[propIdx].propId == PROP_STARTING_LINE ||
        TRACK_PROPS_POS[propIdx].propId == PROP_STONE_STARTING_LINE)
    {
        return;
    }

    TRACK_PROPS_POS[propIdx].propId = kprop_random_prop();

    kui_flag_unsaved_changes();

    return;
}

bool kmaasto_move_prop(const uint propIdx, const vector3<i32> delta)
{
    k_assert(propIdx < TRACK_PROPS_POS.size(), "Tried to move an object whose index was out of bounds. No no no!!");

    // Don't allow moving the starting position.
    if (propIdx == 0)
    {
        return false;
    }

    // Don't allow the object to be moved outside of the track's boundaries.
    if ((TRACK_PROPS_POS[propIdx].pos.x + delta.x) < 0 ||
        (TRACK_PROPS_POS[propIdx].pos.y + delta.y) < 0 ||
        (TRACK_PROPS_POS[propIdx].pos.z + delta.z) < 0 ||
        (TRACK_PROPS_POS[propIdx].pos.x + delta.x) > int(kmaasto_track_width() * kmaasto_track_tile_width()) ||
        (TRACK_PROPS_POS[propIdx].pos.z + delta.z) > int((kmaasto_track_height() + 1) * kmaasto_track_tile_height()))
    {
        return true;
    }

    TRACK_PROPS_POS.at(propIdx).pos.x += delta.x;
    TRACK_PROPS_POS.at(propIdx).pos.y += delta.y;
    TRACK_PROPS_POS.at(propIdx).pos.z += delta.z;

    kui_flag_unsaved_changes();

    return true;
}

void kmaasto_clear_highlights(void)
{
    for (uint i = 0; i < TRACK_PROPS_POS.size(); i++)
    {
        TRACK_PROPS_POS[i].isHighlighted = false;
    }

    HIGHLIGHTED_TILE_COORDS = {-1, -1};

    return;
}

void kmaasto_highlight_ground_tile(const uint tileX, const uint tileZ)
{
    HIGHLIGHTED_TILE_COORDS.x = tileX;
    HIGHLIGHTED_TILE_COORDS.y = tileZ;

    return;
}

void kmaasto_highlight_prop(const uint idx)
{
    k_assert(idx < TRACK_PROPS_POS.size(), "Was asked to highlight an object out of bounds. Won't do it.");

    TRACK_PROPS_POS[idx].isHighlighted = true;

    return;
}

// Change the height of the given tile by a delta from its current value.
//
void kmaasto_change_tile_height(const uint tileX, const uint tileY,
                                const real delta, const int brushSize)
{
    k_assert((tileX <= kmaasto_track_width()),
             "Attempted to set track tile texture out of bounds. Now allowed.");
    k_assert((tileY <= kmaasto_track_height()),
             "Attempted to set track tile texture out of bounds. Now allowed.");

    auto validate_height = [](int &h)
                           {
                               if (h < HEIGHTMAP_VALUE_MIN)
                               {
                                   h = HEIGHTMAP_VALUE_MIN;
                               }
                               else if (h > HEIGHTMAP_VALUE_MAX)
                               {
                                   h = HEIGHTMAP_VALUE_MAX;
                               }
                           };

    if (brushSize == 0) // A size of 0 means the brush covers just one tile.
    {
        if (!kui_is_brush_smoothing())
        {
            const uint idx = (tileX + tileY * TRACK_WIDTH);

            TRACK_HEIGHTMAP[idx] += delta;
            validate_height(TRACK_HEIGHTMAP[idx]);
        }
        else
        {
            /// No smoothing when the brush is only one tile big.
        }
    }
    else
    {
        if (kui_is_brush_smoothing())
        {
            for (int y = -brushSize; y <= brushSize; y++)
            {
                const int yPos = tileY + y;
                if (yPos < 1 || yPos >= (TRACK_WIDTH-1))
                {
                    continue;
                }

                for (int x = -brushSize; x <= brushSize; x++)
                {
                    const int xPos = tileX + x;
                    if (xPos < 1 || xPos >= (TRACK_WIDTH-1))
                    {
                        continue;
                    }

                    const uint idx = (xPos + yPos * TRACK_WIDTH);

                    real avgHeight = 0;
                    avgHeight += TRACK_HEIGHTMAP[(xPos + 1) + (yPos    ) * TRACK_WIDTH];
                    avgHeight += TRACK_HEIGHTMAP[(xPos - 1) + (yPos    ) * TRACK_WIDTH];
                    avgHeight += TRACK_HEIGHTMAP[(xPos    ) + (yPos + 1) * TRACK_WIDTH];
                    avgHeight += TRACK_HEIGHTMAP[(xPos    ) + (yPos - 1) * TRACK_WIDTH];
                    avgHeight += TRACK_HEIGHTMAP[(xPos + 1) + (yPos + 1) * TRACK_WIDTH];
                    avgHeight += TRACK_HEIGHTMAP[(xPos + 1) + (yPos - 1) * TRACK_WIDTH];
                    avgHeight += TRACK_HEIGHTMAP[(xPos - 1) + (yPos + 1) * TRACK_WIDTH];
                    avgHeight += TRACK_HEIGHTMAP[(xPos - 1) + (yPos - 1) * TRACK_WIDTH];
                    avgHeight /= 8;

                    TRACK_HEIGHTMAP[idx] = ((avgHeight + TRACK_HEIGHTMAP[idx]*7) / 8);  /// TODO. Get proper framerate-dependent smoothing.
                    validate_height(TRACK_HEIGHTMAP[idx]);
                }
            }
        }
        else
        {
            for (int y = -brushSize; y <= brushSize; y++)
            {
                const int yPos = tileY + y;
                if (yPos < 0 || yPos >= TRACK_WIDTH)
                {
                    continue;
                }

                for (int x = -brushSize; x <= brushSize; x++)
                {
                    const int xPos = tileX + x;
                    if (xPos < 0 || xPos >= TRACK_WIDTH)
                    {
                        continue;
                    }

                    const uint idx = (xPos + yPos * TRACK_WIDTH);

                    TRACK_HEIGHTMAP[idx] += delta;
                    validate_height(TRACK_HEIGHTMAP[idx]);
                }
            }
        }
    }

    kui_flag_unsaved_changes();

    return;
}

static void update_minimap_texture(void)
{
    generate_topdown_texture();
    TOPDOWN_MAP.make_available_to_renderer();

    return;
}

// Change the texture of the given ground tile.
//
void kmaasto_set_tile_texture(const uint tileX, const uint tileY,
                              const u8 texId, const int brushSize)
{
    if (brushSize == 0) // A size of 0 means one tile only.
    {
        TRACK_TILEMAP[tileX + tileY * TRACK_WIDTH] = texId;
    }
    else
    {
        for (int y = -brushSize; y <= brushSize; y++)
        {
            const int yPos = tileY + y;
            if (yPos < 0 || yPos >= (int)TRACK_WIDTH)
            {
                continue;
            }

            for (int x = -brushSize; x <= brushSize; x++)
            {
                const int xPos = tileX + x;
                if (xPos < 0 || xPos >= (int)TRACK_WIDTH)
                {
                    continue;
                }

                TRACK_TILEMAP[xPos + yPos * TRACK_WIDTH] = texId;
            }
        }
    }

    // Have our changes be reflected on the minimap, as well.
    update_minimap_texture();

    kui_flag_unsaved_changes();

    return;
}

// Loads the MAASTO data from the given file. Note that the file cursor must be
// located where said data begins in the file.
//
static void load_maasto(const file_handle_t fh, const u8 mapId)
{
    DEBUG(("Loading MAASTO data for track #%u...", mapId));

    const u32 fileSize = kfile_read_value<u32>(fh);
    const u32 tilesPerSide = sqrt(fileSize / 2);            // Each height entry is 2 bytes in the file.

    k_assert(((tilesPerSide == 64) || (tilesPerSide == 128)),
             "Was asked to load a MAASTO file with unsupported dimensions (expected 64 x 64 or 128 x 128). Can't do it.");

    MAASTO_FILESIZE = fileSize;
    MAASTO_FILEOFFS = kfile_position(fh);

    load_track_prop_positions(mapId);

    TRACK_HEIGHTMAP.alloc(tilesPerSide * tilesPerSide, "MAASTO heightmap");
    TRACK_TILEMAP.alloc(tilesPerSide * tilesPerSide, "VARIMAA tilemap");

    TRACK_WIDTH = TRACK_HEIGHT = tilesPerSide;

    // Load in the heightmap data.
    bool badValues = false;   // Set to true if we encounter suspicious values.
    for (uint i = 0; i < (tilesPerSide * tilesPerSide); i++)
    {
        const u8 b1 = kfile_read_value<u8>(fh);
        const u8 b2 = kfile_read_value<u8>(fh);

        if (b2 != 0 && b2 != 1 && b2 != 255)
        {
            badValues = true;
        }

        TRACK_HEIGHTMAP[i] = kmaasto_game_height_to_editor_height(b1, b2);
    }
    if (badValues)
    {
        kd_show_headless_warning_message("Invalid height values were encountered when loading the MAASTO "
                                         "data. This may be a corrupted track.");
    }

    return;
}

// Loads the VARIMAA data from the given file. Note that the file cursor must be
// located where said data begins in the file.
//
static void load_varimaa(const file_handle_t fh)
{
    DEBUG(("Loading VARIMAA data..."));

    const u32 fileSize = kfile_read_value<u32>(fh);
    const u32 tileSize = sqrt(fileSize);                // How many tiles per side there are. Assumes a square.

    k_assert((tileSize == TRACK_WIDTH),
             "The VARIMAA file's dimensions differ from the corresponding MAASTO file. Not good.");

    k_assert(((tileSize == 64) || (tileSize == 128)),
             "Was asked to load a VARIMAA file with unsupported dimensions (expected 64 x 64 or 128 x 128).");

    VARIMAA_FILESIZE = fileSize;
    VARIMAA_FILEOFFS = kfile_position(fh);

    kfile_read_byte_array(TRACK_TILEMAP.ptr(), TRACK_TILEMAP.up_to(VARIMAA_FILESIZE), fh);

    return;
}

static void load_checkpoint(void)
{
    const u32 offs = kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_TRACK_HEADER_BLOCK, kmanif_track_idx());

    kfile_seek((offs + 13), kge_file_handle_rallye_exe());
    TRACK_CHECKPOINT.x = kfile_read_value<u8>(kge_file_handle_rallye_exe());
    TRACK_CHECKPOINT.x *= 2;

    kfile_seek((offs + 15), kge_file_handle_rallye_exe());
    TRACK_CHECKPOINT.y = kfile_read_value<u8>(kge_file_handle_rallye_exe());
    TRACK_CHECKPOINT.y *= 2;

    return;
}

/// Temp hack. Creates a tile that represents the water level. This should be better
/// integrated into the code.
static void create_water_level_tile(void)
{
    const uint waterTileSideLen = 16;

    #define Y 10
    const u8 waterTile[waterTileSideLen * waterTileSideLen] =
            {0,0,0,0,0,0,0,0,0,0,0,Y,0,0,0,0,
             0,0,0,Y,0,0,0,0,0,0,Y,0,0,0,0,0,
             0,0,Y,0,0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,Y,0,0,0,0,0,0,0,
             Y,0,0,0,0,0,0,Y,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,0,0,0,0,0,0,Y,0,
             0,0,0,0,0,Y,0,0,0,0,0,0,0,Y,0,0,
             0,0,0,0,Y,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,0,0,0,Y,0,0,0,0,
             0,0,Y,0,0,0,0,0,0,0,Y,0,0,0,0,0,
             0,Y,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,Y,0,0,0,0,0,0,Y,
             0,0,0,0,0,0,0,Y,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,0,0,0,0,0,Y,0,0,
             0,0,0,0,0,Y,0,0,0,0,0,0,Y,0,0,0,
             0,0,0,0,Y,0,0,0,0,0,0,0,0,0,0,0};
    #undef Y


    WATER_LEVEL_TEX.initialize({waterTileSideLen, waterTileSideLen, 8}, "Water level texture");
    WATER_LEVEL_TEX.isFiltered = false;
    memcpy(WATER_LEVEL_TEX.pixels.ptr(), waterTile, WATER_LEVEL_TEX.pixels.up_to(sizeof(waterTile)));
    WATER_LEVEL_TEX.make_available_to_renderer();

    return;
}

// Main entry. Loads the MAASTO and VARIMAA data from the given file (assumed to
// be a RallySportED project container). Assumes that the file's cursor is positioned
// correctly for these load operations to begin without further positioning, and that
// the two blocks of data occur in this order without intervening data.
//
void kmaasto_initialize_maasto(const file_handle_t fileHandle, const u8 mapId)
{
    k_assert((TRACK_HEIGHTMAP.is_null()),
             "Was asked to initialize the MAASTO, but the MAASTO buffer was not NULL. Can't overwrite existing data.");

    SOURCE_FILE_HANDLE = fileHandle;

    load_maasto(SOURCE_FILE_HANDLE, mapId);
    load_varimaa(SOURCE_FILE_HANDLE);
    load_checkpoint();

    create_water_level_tile();
    kfile_seek(kge_game_exe_offset(GE_RALLYE_EXE, GE_OFFS_TRACK_WATER_LEVEL, kmanif_track_idx()), kge_file_handle_rallye_exe());
    WATER_LEVEL = -kfile_read_value<u8>(kge_file_handle_rallye_exe());

  //  DEBUG(("Loaded MAASTO data from '%s' (%u x %u tiles).", maastoFilename, tileSize, tileSize));

    return;
}

// Create a topdown 2d texture of the track's tilemap.
//
void kmaasto_generate_topdown_view(void)
{
    if (TOPDOWN_MAP.pixels.is_null())   // If first-time use, initialize.
    {
        TOPDOWN_MAP.initialize({kmaasto_track_width(), kmaasto_track_height(), 8}, "Topdown map texture");
        TOPDOWN_MAP.isFiltered = false;
    }

    generate_topdown_texture();
    TOPDOWN_MAP.make_available_to_renderer();

    return;
}

// Append a triangle mesh of the heightmap from the current position of the scene's
// camera.
//
void kmaasto_add_maasto_tri_mesh(std::vector<triangle_s> *const dstMesh)
{
    const int px = kcamera_tile_position_x();
    const int py = kcamera_tile_position_y();

    const uint w = kmaasto_track_tile_width();
    const uint h = kmaasto_track_tile_height();

    const uint numX = kmaasto_maasto_draw_tile_width();
    const uint numY = kmaasto_maasto_draw_tile_height();

    const int startX = px * w;
    const int endX = startX + (numX * w);
    const int startZ = py * h;
    const int endZ = startZ + (numY * h) + (h * 5); // Add h * 5 to avoid having 3d objects 'pop' onto the screen at the bottom when scrolling.

    // Add the triangles for the ground and billboards.
    for (uint z = 0; z < numY; z++)
    {
        // Ground tiles.
        for (uint x = 0; x < numX; x++)
        {
            const int posX = px + x - 13;
            const int posY = py + z - 34;// - numY*1.3;

            triangle_s t;
            t.interact.type = INTERACTIBLE_GROUND;
            t.interact.params[INTERACT_PARAM_GROUND_X] = x;
            t.interact.params[INTERACT_PARAM_GROUND_Z] = z;
            t.interact.params[INTERACT_PARAM_GROUND_X_GLOBAL] = (px + x);
            t.interact.params[INTERACT_PARAM_GROUND_Z_GLOBAL] = (py + z);
            t.interact.params[INTERACT_PARAM_GROUND_IS_MAIN] = 1;

            u8 texture = texture_idx_at(px + x, py + z);

            int heightFrontRight = -kmaasto_height_at(px + x + 1, py + z);
            int heightBackRight = -kmaasto_height_at(px + x + 1, py + z + 1);
            int heightFrontLeft = -kmaasto_height_at(px + x, py + z);
            int heightBackLeft = -kmaasto_height_at(px + x, py + z + 1);

            // Adjust the height of the terrain to match the water level.
            if (kui_is_real_water_level_enabled() &&
                ((px + x) > 0) && ((px + x) < (kmaasto_track_tile_width() - 1)) &&    // <- Test that we don't apply water level to corner tiles.
                ((py + z) > 0) && ((py + z) < (kmaasto_track_tile_height() - 1)))
            {
                // The way the adjustment works is, if the current texture is 0,
                // i.e. water, all of the tile's corners should be set at the water
                // level. Tile corners adjacent to water tiles should likewise be
                // set flush with the water level.

                if (texture == 0)
                {
                    heightFrontRight = -WATER_LEVEL;
                    heightBackRight = -WATER_LEVEL;
                    heightFrontLeft = -WATER_LEVEL;
                    heightBackLeft = -WATER_LEVEL;
                }

                // Corners.
                if (texture_idx_at(px + x, py + z - 1) == 0)        // Front.
                {
                    heightFrontRight = -WATER_LEVEL;
                    heightFrontLeft = -WATER_LEVEL;
                }
                if (texture_idx_at(px + x, py + z + 1) == 0)        // Back.
                {
                    heightBackRight = -WATER_LEVEL;
                    heightBackLeft = -WATER_LEVEL;
                }
                if (texture_idx_at(px + x - 1, py + z) == 0)        // Left.
                {
                    heightFrontLeft = -WATER_LEVEL;
                    heightBackLeft = -WATER_LEVEL;
                }
                if (texture_idx_at(px + x + 1, py + z) == 0)        // Right.
                {
                    heightFrontRight = -WATER_LEVEL;
                    heightBackRight = -WATER_LEVEL;
                }
                if (texture_idx_at(px + x + 1, py + z - 1) == 0)    // Front right.
                {
                    heightFrontRight = -WATER_LEVEL;
                }
                if (texture_idx_at(px + x - 1, py + z - 1) == 0)    // Front left.
                {
                    heightFrontLeft = -WATER_LEVEL;
                }
                if (texture_idx_at(px + x + 1, py + z + 1) == 0)    // Back right.
                {
                    heightBackRight = -WATER_LEVEL;
                }
                if (texture_idx_at(px + x - 1, py + z + 1) == 0)    // Back left.
                {
                    heightBackLeft = -WATER_LEVEL;
                }
            }

            // Highlight the ground tile that's under the cursor.
            /*if ((HIGHLIGHTED_TILE_COORDS.x != -1) &&
                (abs((int)x - HIGHLIGHTED_TILE_COORDS.x) < kui_brush_size()) &&
                (abs((int)z - HIGHLIGHTED_TILE_COORDS.y) < kui_brush_size()) &&
                !kui_is_editing_terrain() &&
                !kcamera_camera_is_moving())*/
            if ((int)x == HIGHLIGHTED_TILE_COORDS.x &&
                (int)z == HIGHLIGHTED_TILE_COORDS.y &&
                !kui_is_editing_terrain() &&
                !kcamera_camera_is_moving())
            {
                texture = kui_brush_pala_idx();
            }

            t.baseColor = kpalat_pala_ptr(texture)->pixels[1];
            t.texturePtr = kpalat_pala_ptr(texture);

            t.v[0].w = 1;
            t.v[1].w = 1;
            t.v[2].w = 1;

            // Left triangle.
            {
                t.v[0].x = (posX * w);
                t.v[0].y = heightFrontLeft;
                t.v[0].z = (posY * h);
                t.v[0].uv[0] = (0);
                t.v[0].uv[1] = (15.9);

                t.v[1].x = (posX * w);
                t.v[1].y = heightBackLeft;
                t.v[1].z = (posY * h) + h;
                t.v[1].uv[0] = (0);
                t.v[1].uv[1] = (0);

                t.v[2].x = (posX * w) + w;
                t.v[2].y = heightBackRight;
                t.v[2].z = (posY * h) + h;
                t.v[2].uv[0] = (15.9);
                t.v[2].uv[1] = (0);

                t.interact.params[INTERACT_PARAM_GROUND_TWIN] = (dstMesh->size() + 1);
                dstMesh->push_back(t);
            }

            // Right triangle.
            {
                t.interact.params[INTERACT_PARAM_GROUND_IS_MAIN] = 0;

                t.v[0].x = (posX * w);
                t.v[0].y = heightFrontLeft;
                t.v[0].z = (posY * h);
                t.v[0].uv[0] = (0);
                t.v[0].uv[1] = (15.9);

                t.v[1].x = (posX * w) + w;
                t.v[1].y = heightBackRight;
                t.v[1].z = (posY * h) + h;
                t.v[1].uv[0] = (15.9);
                t.v[1].uv[1] = (0);

                t.v[2].x = (posX * w) + w;
                t.v[2].y = heightFrontRight;
                t.v[2].z = (posY * h);
                t.v[2].uv[0] = (15.9);
                t.v[2].uv[1] = (15.9);

                t.interact.params[INTERACT_PARAM_GROUND_TWIN] = (dstMesh->size() - 1);
                dstMesh->push_back(t);
            }
        }

        // Loop through this x slice again to add any billboards (tile-bound 2d
        // elements like spectators and bushes). We do this as a separate step to
        // prevent depth clipping issues between adjacent triangles when not depth-
        // sorting the mesh.
        for (uint x = 0; x < numX; x++)
        {
            const uint posX = px + x - 13;
            const uint posY = py + z - 34;//numY*1.3;

            triangle_s t;
            t.interact.type = INTERACTIBLE_IGNORE;

            u8 texture = texture_idx_at(px + x, py + z);

            if (HIGHLIGHTED_TILE_COORDS.x != -1 &&
                abs((int)x - HIGHLIGHTED_TILE_COORDS.x) < kui_brush_size() &&
                abs((int)z - HIGHLIGHTED_TILE_COORDS.y) < kui_brush_size() &&
                !kui_is_editing_terrain() &&
                !kcamera_camera_is_moving())
            /*if ((int)x == HIGHLIGHTED_TILE_COORDS.x &&
                (int)z == HIGHLIGHTED_TILE_COORDS.y &&
                !kui_is_editing_terrain() &&
                !kcamera_camera_is_moving())*/
            {
                texture = kui_brush_pala_idx();
            }

            t.baseColor = TEXTURE_ALPHA_ENABLED;

            t.v[0].w = 1;
            t.v[1].w = 1;
            t.v[2].w = 1;

            // If the tile is water, indicate the water level with a custom graphic
            // above the tile.
            if (!kui_is_real_water_level_enabled() &&
                texture == 0)
            {
                const int baseHeight = -WATER_LEVEL;

                // Water tiles always occur at a height of 0 or less, so ignore those that
                // would fall underground.
                if (baseHeight >= 0)
                {
                    t.texturePtr = &WATER_LEVEL_TEX;

                    // Left triangle.
                    {
                        t.v[0].x = (posX * w);
                        t.v[0].y = baseHeight;
                        t.v[0].z = (posY * h);
                        t.v[0].uv[0] = (0);
                        t.v[0].uv[1] = (15.9);

                        t.v[1].x = (posX * w);
                        t.v[1].y = baseHeight;
                        t.v[1].z = (posY * h) + h;
                        t.v[1].uv[0] = (0);
                        t.v[1].uv[1] = (0);

                        t.v[2].x = (posX * w) + w;
                        t.v[2].y = baseHeight;
                        t.v[2].z = (posY * h) + h;
                        t.v[2].uv[0] = (15.9);
                        t.v[2].uv[1] = (0);

                        dstMesh->push_back(t);
                    }

                    // Right triangle.
                    {
                        t.v[0].x = (posX * w);
                        t.v[0].y = baseHeight;
                        t.v[0].z = (posY * h);
                        t.v[0].uv[0] = (0);
                        t.v[0].uv[1] = (15.9);

                        t.v[1].x = (posX * w) + w;
                        t.v[1].y = baseHeight;
                        t.v[1].z = (posY * h) + h;
                        t.v[1].uv[0] = (15.9);
                        t.v[1].uv[1] = (0);

                        t.v[2].x = (posX * w) + w;
                        t.v[2].y = baseHeight;
                        t.v[2].z = (posY * h);
                        t.v[2].uv[0] = (15.9);
                        t.v[2].uv[1] = (15.9);

                        dstMesh->push_back(t);
                    }
                }
                else
                {
                    continue;
                }
            }
            // If the tile has a bridge, add that.
            else if (texture == 248 || texture == 249)
            {
                std::list<int> cornerHeights;
                cornerHeights.push_back(kmaasto_height_at(px + x,     py + z));
                cornerHeights.push_back(kmaasto_height_at(px + x + 1, py + z));
                cornerHeights.push_back(kmaasto_height_at(px + x + 1, py + z + 1));
                cornerHeights.push_back(kmaasto_height_at(px + x,     py + z + 1));
                cornerHeights.sort();
                const int baseHeight = cornerHeights.front();

                // Bridge tiles always occur at a height of 0, so only put them
                // on tiles where the ground is below that.
                if (baseHeight < 0)
                {
                    t.texturePtr = kpalat_pala_ptr(177);    // Use the generic bridge texture.

                    // Left triangle.
                    {
                        t.v[0].x = (posX * w);
                        t.v[0].y = 0;
                        t.v[0].z = (posY * h);
                        t.v[0].uv[0] = (0);
                        t.v[0].uv[1] = (15.9);

                        t.v[1].x = (posX * w);
                        t.v[1].y = 0;
                        t.v[1].z = (posY * h) + h;
                        t.v[1].uv[0] = (0);
                        t.v[1].uv[1] = (0);

                        t.v[2].x = (posX * w) + w;
                        t.v[2].y = 0;
                        t.v[2].z = (posY * h) + h;
                        t.v[2].uv[0] = (15.9);
                        t.v[2].uv[1] = (0);

                        dstMesh->push_back(t);
                    }

                    // Right triangle.
                    {
                        t.v[0].x = (posX * w);
                        t.v[0].y = 0;
                        t.v[0].z = (posY * h);
                        t.v[0].uv[0] = (0);
                        t.v[0].uv[1] = (15.9);

                        t.v[1].x = (posX * w) + w;
                        t.v[1].y = 0;
                        t.v[1].z = (posY * h) + h;
                        t.v[1].uv[0] = (15.9);
                        t.v[1].uv[1] = (0);

                        t.v[2].x = (posX * w) + w;
                        t.v[2].y = 0;
                        t.v[2].z = (posY * h);
                        t.v[2].uv[0] = (15.9);
                        t.v[2].uv[1] = (15.9);

                        dstMesh->push_back(t);
                    }
                }
                else
                {
                    continue;
                }
            }
            // If the tile has a billboard, add that too.
            else if (texture > 239 && texture < 248)
            {
                const uint billHeight = h;
                int baseHeight = -kmaasto_height_at(px + x, py + z);

                if (kui_is_real_water_level_enabled() &&
                    (px + x) != 0 && (px + x) != (kmaasto_track_tile_width() - 1) &&
                    (py + z) != 0 && (py + z) != (kmaasto_track_tile_height() - 1))
                {
                    // Adjust billboard heights to match the water level.
                    if (texture_idx_at(px + x, py + z - 1) == 0 ||
                        texture_idx_at(px + x, py + z + 1) == 0 ||
                        texture_idx_at(px + x - 1, py + z) == 0 ||
                        texture_idx_at(px + x + 1, py + z) == 0 ||
                        texture_idx_at(px + x + 1, py + z - 1) == 0 ||
                        texture_idx_at(px + x - 1, py + z - 1) == 0 ||
                        texture_idx_at(px + x + 1, py + z + 1) == 0 ||
                        texture_idx_at(px + x - 1, py + z + 1) == 0)
                    {
                        baseHeight = -WATER_LEVEL;
                    }
                }

                switch (texture)
                {
                    // Spectators.
                    case 240:
                    case 241:
                    case 242: { t.texturePtr = kpalat_pala_ptr(spectator_texture_at((px + x), (py + z))); break; }

                    // Shrubs.
                    case 243: { t.texturePtr = kpalat_pala_ptr(208); break; }
                    case 244: { t.texturePtr = kpalat_pala_ptr(209); break; }
                    case 245: { t.texturePtr = kpalat_pala_ptr(210); break; }

                    // Small poles.
                    case 246:
                    case 247: { t.texturePtr = kpalat_pala_ptr(211); break; }
                    case 250: { t.texturePtr = kpalat_pala_ptr(212); break; }

                    // No recognized billboard.
                    default: k_assert(0, "Unrecognized billboard texture."); continue;
                }

                // Left triangle.
                {
                    t.v[0].x = (posX * w);
                    t.v[0].y = baseHeight - billHeight;
                    t.v[0].z = (posY * h) + h;
                    t.v[0].uv[0] = (0);
                    t.v[0].uv[1] = (16);

                    t.v[1].x = (posX * w);
                    t.v[1].y = baseHeight;
                    t.v[1].z = (posY * h) + h;
                    t.v[1].uv[0] = (0);
                    t.v[1].uv[1] = (1);

                    t.v[2].x = (posX * w) + w;
                    t.v[2].y = baseHeight;
                    t.v[2].z = (posY * h) + h;
                    t.v[2].uv[0] = (15);
                    t.v[2].uv[1] = (1);

                    dstMesh->push_back(t);
                }

                // Right triangle.
                {
                    t.v[0].x = (posX * w);
                    t.v[0].y = baseHeight - billHeight;
                    t.v[0].z = (posY * h) + h;
                    t.v[0].uv[0] = (0);
                    t.v[0].uv[1] = (16);

                    t.v[1].x = (posX * w) + w;
                    t.v[1].y = baseHeight;
                    t.v[1].z = (posY * h) + h;
                    t.v[1].uv[0] = (15);
                    t.v[1].uv[1] = (1);

                    t.v[2].x = (posX * w) + w;
                    t.v[2].y = baseHeight - billHeight;
                    t.v[2].z = (posY * h) + h;
                    t.v[2].uv[0] = (15);
                    t.v[2].uv[1] = (16);

                    dstMesh->push_back(t);
                }
            }
        }
    }

    // Find which 3d objects are within the range of tiles we were asked for.
    for (uint i = 0; i < TRACK_PROPS_POS.size(); i++)
    {
        const track_prop_s &prop = TRACK_PROPS_POS[i];

        if (prop.pos.x >= startX && prop.pos.x <= endX &&
            prop.pos.z >= startZ && prop.pos.z <= endZ)
        {
            const int height = -kmaasto_height_at(prop.pos.x / kmaasto_track_tile_width(),
                                                  prop.pos.z / kmaasto_track_tile_height());

            kprop_add_prop_mesh(prop, dstMesh, height);
        }
    }

    return;
}

void kmaasto_release_maasto(void)
{
    TRACK_HEIGHTMAP.release_memory();
    TRACK_TILEMAP.release_memory();
    TOPDOWN_MAP.pixels.release_memory();
    WATER_LEVEL_TEX.pixels.release_memory();

    return;
}

// Returns the number of ground tiles that will be added horizontally when
// kmaasto_add_maasto_tri_mesh() is called.
//
uint kmaasto_maasto_draw_tile_width(void)
{
    return 26;
}
uint kmaasto_maasto_draw_tile_height(void)
{
    return 26;
}

// Returns the number of tiles on the track horizontally.
//
uint kmaasto_track_width(void)
{
    return TRACK_WIDTH;
}

uint kmaasto_track_height(void)
{
    return TRACK_HEIGHT;
}

// Returns the world-unit width of a ground tile.
//
uint kmaasto_track_tile_width(void) /// TODO. Needs a better name to reflect the fact that we're talking about rendering/world-unit widths.
{
    return WIDTH_OF_TILE;
}
uint kmaasto_track_tile_height(void)
{
    return HEIGHT_OF_TILE;
}

const texture_c* kmaasto_track_topdown_texture_ptr(void)
{
    return &TOPDOWN_MAP;
}
