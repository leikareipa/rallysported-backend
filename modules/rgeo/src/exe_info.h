#ifndef EXE_INFO_H_
#define EXE_INFO_H_

#include "types.h"

/* How many tracks we expect Rally-Sport to have. The demo has eight.*/
#define KEXE_NUM_TRACKS 8

u32 kexe_rallye_track_header_block_start(const u8 trackIdx);

u32 kexe_valikko_track_header_block_start(const u8 trackIdx);

u32 kexe_rallye_prop_block_start(const u8 trackIdx);

u32 kexe_rallye_prop_block_end(const u8 trackIdx);

u32 kexe_rallye_starting_position_block(const u8 trackIdx);

u32 kexe_rallye_prop_block_length(void);

u32 kexe_rallye_default_prop_count(const u8 trackIdx);

u32 kexe_rallye_prop_offset(const u8 propIdx, const u8 trackIdx);

u32 kexe_rallye_executable_file_size(void);

const u8* kexe_rallye_prop_type_id(const u8 propType);

#endif
