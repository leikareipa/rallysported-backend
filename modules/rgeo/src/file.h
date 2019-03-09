#ifndef FILE_H_
#define FILE_H_

#include "types.h"

void kf_read_bytes(u8 *dst, const u32 numBytes, const file_handle handle);

file_handle kf_open_file(const char *const filename, const char *const openMode);

int kf_is_active_handle(const file_handle handle);

void kf_close_file(const file_handle handle);

long kf_file_size(const file_handle handle);

#endif
