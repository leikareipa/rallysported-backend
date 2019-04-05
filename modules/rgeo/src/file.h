#ifndef FILE_H_
#define FILE_H_

#include "types.h"

/* For mkdir().*/
#if __WATCOMC__
    #include <direct.h>
#else
    #include <sys/stat.h>
#endif

/* A guaranteed unused handle.*/
#define KF_INVALID_HANDLE (file_handle)~0u

#if __unix__
    #define kf_create_directory(dirName) mkdir((dirName), S_IRWXU | S_IRWXG | S_IROTH)
#else
    #define kf_create_directory(dirName) mkdir((dirName))
#endif

int kf_buffered_data_copy(const file_handle srcHandle, const file_handle dstHandle);

void kf_read_bytes(u8 *dst, const u32 numBytes, const file_handle handle);

file_handle kf_open_file(const char *const filename, const char *const openMode);

int kf_is_valid_handle(const file_handle handle);

void kf_close_file(const file_handle handle);

u32 kf_file_size(const file_handle handle);

void kf_jump(const u32 pos, const file_handle handle);

#endif
