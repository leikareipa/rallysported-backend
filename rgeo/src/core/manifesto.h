/*
 * 2016-2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef MANIFESTO_H
#define MANIFESTO_H

#include "../core/types.h"

enum manifesto_command_e
{
    MANIFCMD_NONE = -1,
    MANIFCMD_REQUIRE = 0,
    MANIFCMD_ROAD = 1,
    MANIFCMD_NUM_PROPS = 2,
    MANIFCMD_ADD_PROP = 3,
    MANIFCMD_SET_PROP = 4,
    MANIFCMD_MOVE_PROP = 5,
    MANIFCMD_MOVE_STARTING_POS = 6,
    MANIFCMD_SET_COLOR = 10,
    MANIFCMD_STOP = 99
};

bool kmanif_apply_manifesto(const char *const manifestoFilename, const file_handle_t targetFh);

u8 kmanif_track_idx(void);

void kmanif_save_manifesto_file(void);

void kmanif_add_command_to_manifesto(const manifesto_command_e cmd, const u8 *const params);

#endif
