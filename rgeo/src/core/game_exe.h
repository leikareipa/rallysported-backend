/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef GAME_EXE_H
#define GAME_EXE_H

#include "types.h"

/// Temp.
const uint RALLYE_NUM_BYTES_IN_PROP_BLOCK = 12; // Number of bytes in a track's prop header block.

enum exe_id_e
{
    GE_RALLYE_EXE,
    GE_VALIKKO_EXE
};

enum exe_offset_e
{
    GE_OFFS_PALETTE,
    GE_OFFS_TRACK_PROP_BLOCK,
    GE_OFFS_TRACK_PROP_BLOCK_END,
    GE_OFFS_TRACK_HEADER_BLOCK,
    GE_OFFS_TRACK_WATER_LEVEL,
    GE_OFFS_DATA_SEGMENT,
    GE_OFFS_TRACK_PROP,
    GE_OFFS_STARTING_POSITION,
    GE_OFFS_STRING_LOCATION_MAASTO,
    GE_OFFS_STRING_LOCATION_VARIMAA,
    GE_OFFS_STRING_LOCATION_PALAT,
    GE_OFFS_PROP_TEXTURE_UVS      // 3d track object textures' uv coordinates for TEXT1.DTA.
};

void kge_acquire_access_to_game_executables(const char * const fnRallye, const char * const fnValikko);

void kge_prop_id_string(u8 *const str, const u8 propIdx);

file_handle_t kge_file_handle_rallye_exe(void);

file_handle_t kge_file_handle_valikko_exe(void);

u32 kge_game_exe_offset(const exe_id_e exe, const exe_offset_e offsType, const u8 param = 0, const u8 param2 = 0);

#endif
