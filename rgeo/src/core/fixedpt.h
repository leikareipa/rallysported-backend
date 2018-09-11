/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 * Some helper macros for fixed-point messing-with.
 *
 */

#ifndef FIXEDPT_H
#define FIXEDPT_H

#include <cstdlib>
#include <cmath>

// Use half and half of the variable.
#define FIXEDP(var)       ((var) << (sizeof(var) << 2))
#define UN_FIXEDP(var)    ((var) >> (sizeof(var) << 2))

/// TODO.
//#define FIXEDP_UV(var)       ((var) << 20)
//#define UN_FIXEDP_UV(var)    ((var) >> 20)

// For angles using u16; where 0 = 0 deg and ~0 = 359 deg.
#define FP_ROT_DEG(deg) u16(roundl((u16(~0u) / 360.0) * \
                                   ((deg) < 0? (360 - (abs(deg) % 360)) : ((deg) % 360))  ))  // Turn the given angle into its corresponding fixed-point version.
#define FP_UNPACK_FOR_SINE_LUT(fp) ((fp) >> 6)  // Shift the fixed-point value to where it can be used to index into a sine lookup table.

#endif
