/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef PROPS_H
#define PROPS_H

#include <vector>
#include "../core/ui.h"

// Stores the relative byte offsets in RALLYE.EXE for the various 3d track props.
// The first value, or the high 16 bits, is the offset to the prop's vertex
// coordinates, and the second value to the vertex indices. The bit combination
// of these two offsets uniquely identifies each prop.
enum prop_id_e : u32
{
    PROP_TREE                        = ((0xd02cu << 16) | 0xd088u),
    PROP_WIRE_FENCE                  = ((0x47e2u << 16) | 0x4c98u),
    PROP_HORSE_FENCE                 = ((0x47e2u << 16) | 0x4d98u),
    PROP_TRAFFIC_SIGN_80             = ((0x4766u << 16) | 0x4c20u),
    PROP_TRAFFIC_SIGN_EXCLAMATION    = ((0x4766u << 16) | 0x4c5cu),
    PROP_STONE_ARCH                  = ((0x4ff2u << 16) | 0x51e0u),
    PROP_STONE_POST                  = ((0x4abau << 16) | 0x4aecu),
    PROP_LARGE_ROCK                  = ((0x4932u << 16) | 0x49e4u),
    PROP_SMALL_ROCK                  = ((0x498eu << 16) | 0x49e4u),
    PROP_LARGE_BILLBOARD             = ((0x4466u << 16) | 0x4560u),
    PROP_SMALL_BILLBOARD             = ((0x44e6u << 16) | 0x4660u),
    PROP_BUILDING                    = ((0x48eeu << 16) | 0x4b7cu),
    PROP_UTIL_POLE_1                 = ((0x4324u << 16) | 0x434au),
    PROP_UTIL_POLE_2                 = ((0x4324u << 16) | 0x439cu),
    PROP_STARTING_LINE               = ((0x5488u << 16) | 0x5502u),
    PROP_STONE_STARTING_LINE         = ((0x50d2u << 16) | 0x51c4u),
};

// A single 3d track prop (trees, houses, stone arches, etc.).
struct track_prop_s
{
    vector3<i32> pos;           // Position on the track (in world coordinates, not tile ones).
    uint idx;                   // The index of this prop in the master prop array.
    u32 propId;                 // The prop_id_e enum cast into u32.
    bool isHighlighted;         // Whether this prop is currently highlighted, e.g. by having the mouse cursor hovering over it.
};

prop_id_e kprop_random_prop(void);

uint kprop_prop_idx_for_type(const prop_id_e propType);

void kprop_initialize_props(void);

void kprop_release_props(void);

void kprop_add_prop_mesh(const track_prop_s &prop, std::vector<triangle_s> * const dstMesh, const int terrainHeight);

#endif
