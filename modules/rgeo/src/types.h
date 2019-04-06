/*
 * 2018, 2019 Tarpeeksi Hyvae Soft
 * 
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

typedef int32_t      i32;
typedef int16_t      i16;
typedef int8_t       i8;
typedef uint32_t     u32;
typedef uint16_t     u16;
typedef uint8_t      u8;

typedef unsigned int uint;
typedef float        real;
typedef u8           palette_idx; /* Color index to a 256-color palette.*/
typedef u16          file_handle; /* File id returned kf_open_binary_file() in file.c.*/

#endif
