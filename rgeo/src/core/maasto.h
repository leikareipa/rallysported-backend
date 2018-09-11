/*
 * 2017, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef MAASTO_H
#define MAASTO_H

#include <vector>
#include "types.h"

struct triangle_s;
struct triangle_mesh_s;
template <typename T> struct vector3;

const texture_c* kmaasto_track_topdown_texture_ptr(void);

void kmaasto_initialize_maasto(const file_handle_t fileHandle, const u8 mapId);

uint kmaasto_track_width(void);

int kmaasto_game_height_to_editor_height(const u8 b1, const u8 b2);

std::pair<u8, u8> kmaasto_editor_height_to_game_height(const int h);

bool kmaasto_save_track(void);

void kmaasto_release_maasto(void);

int kmaasto_height_at(const uint x, const uint y);

u8 kmaasto_texture_at(const uint x, const uint y);

uint kmaasto_maasto_draw_tile_width(void);

uint kmaasto_maasto_draw_tile_height(void);

uint kmaasto_track_width(void);

uint kmaasto_track_height(void);

void kmaasto_clear_highlights(void);

void kmaasto_change_prop_type(const uint propIdx);

bool kmaasto_move_prop(const uint propIdx, const vector3<i32> delta);

void kmaasto_generate_topdown_view(void);

void kmaasto_highlight_prop(const uint idx);

void kmaasto_set_tile_texture(const uint tileX, const uint tileY, const u8 texId, const int brushSize);

void kmaasto_change_tile_height(const uint tileX, const uint tileY, const real delta, const int brushSize);

void kmaasto_highlight_ground_tile(const uint tileX, const uint tileZ);

void kmaasto_add_maasto_tri_mesh(std::vector<triangle_s> * const dstMesh);

uint kmaasto_track_tile_width(void);

uint kmaasto_track_tile_height(void);

#endif
