/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef TYPES_H
#define TYPES_H

#include "stdint.h"

typedef int64_t         i64;
typedef int32_t         i32;
typedef int16_t         i16;
typedef int8_t          i8;
typedef uint64_t        u64;
typedef uint32_t        u32;
typedef uint16_t        u16;
typedef uint8_t         u8;

typedef unsigned int    uint;
typedef float           real;
typedef u8              palette_idx_t;
typedef u16             file_handle_t;

typedef i32             fixedpoint_t;   // Meant for when 32-bit fixed-point processing is to be done on the variable.

#endif
