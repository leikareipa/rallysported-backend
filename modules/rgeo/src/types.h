/*
 * 2018, 2019 Tarpeeksi Hyvae Soft
 * 
 */

#ifndef TYPES_H_
#define TYPES_H_

/* For 16-bit DOS.*/
typedef long          i32;
typedef int           i16;
typedef char          i8;
typedef unsigned long u32;
typedef unsigned int  u16;
typedef unsigned char u8;

typedef u16           uint;
typedef float         real;
typedef u8            palette_idx; /* Color index to a 256-color palette.*/
typedef u16           file_handle; /* File id returned kf_open_binary_file() in file.c.*/

#endif
