/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include "types.h"

#ifdef NDEBUG
    #error "NDEBUG disables assertions. Assertions are required by design."
#endif
#include <cassert>

#pragma pack(push, 1)
    struct color_bgra_s
    {
        u8 b, g, r, a;
    };

    struct color_rgb_s
    {
        u8 r, g, b;
    };
#pragma pack(pop)

struct resolution_s
{
    uint w, h, bpp;
};

// The version number of this program.
const int INTERNAL_VERSION_MAJOR = 8;
const int INTERNAL_VERSION_MINOR = 5;

// The minimum RSED Loader version we require for maps made with this version of the editor.
const char REQUIRED_LOADER_VER = '3';
const int LOADER_MAJOR_VERSION = 3;

// The underlying, orignal resolution of Rally-Sport.
const resolution_s NATIVE_RESOLUTION = {320, 200, 8};

// These should be set so as to allow compatibility with some old renderers,
// like Glide (which has 256 x 256 max).
const uint MAX_TEXTURE_WIDTH = 256;
const uint MAX_TEXTURE_HEIGHT = 256;

// How many tracks there is in Rally-Sport by default. This assumes the demo version.
const uint NUM_GAME_TRACKS = 8;

#ifdef _WIN32
    const char DIR_SEPARATOR = '\\';
#else
    const char DIR_SEPARATOR = '/';
#endif

// The ratio of the game's original resolution to the editor's display resolution,
// for convenience.
#define NATIVE_RES_MUL_X (kd_display_resolution().w / (real)NATIVE_RESOLUTION.w)
#define NATIVE_RES_MUL_Y (kd_display_resolution().h / (real)NATIVE_RESOLUTION.h)

#define INFO(args)          (printf("[INFO ] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))
#define ERRORI(args)        (fprintf(stderr, "[ERROR] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))
#define DEBUG(args)         (printf("[DEBUG] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))

#define k_assert(condition, error_string)   if (!(condition))\
                                            {\
                                                kd_show_headless_assert_error_message(error_string); /*Notify in a user-friendly way.*/\
                                                assert(condition && error_string);\
                                            }

// For assertions in performance-critical sections; not guaranteed to evaluate
// to an assertion in release-oriented buids.
#ifdef RSED_ON_QT // For now, enable extra assertions on the dev version only.
    #define k_assert_optional k_assert
#else
    #define k_assert_optional(...)
#endif

// Validation runs are automated testings of particular aspects of the program's
// functioning.
#ifdef VALIDATION_RUN
    #undef k_assert
    #define k_assert(condition, error_string)   if (!(condition))\
                                                {\
                                                    throw error_string;\
                                                }

    // Disable standard console output, for cleaner usage.
    #undef DEBUG
    #undef ERRORI
    #undef INFO
    #define DEBUG(...)
    #define ERRORI(...)
    #define INFO(...)
#endif

// For a lookup table on sin/cos.
extern const real sin_lut[];
#define SIN(x) (sin_lut[(x)])
#define COS(x) (sin_lut[(x) + 256])

#define LERP(x, y, step)    ((x) + ((step) * ((y) - (x))))

#define NUM_ELEMENTS(array) int((sizeof(array) / sizeof((array)[0])))

#define CLAMP_VALUE(val, min, max) {if ((val) < (min)) (val) = (min); else if ((val) > (max)) (val) = (max);}

// Converts the given frame rate into milliseconds per frame.
#define FRAME_RATE(x) (((x) == 0)? 0 : (1000 / (x)))

const int RAYTRACE_NO_HIT = -1;             // Value for when ray-tracing hit nothing. Used for e.g. mouse-picking.

const uint TEXTURE_ALPHA_ENABLED = 255;     // Used to signal that a given texture should have alpha testing.

const uint ALPHA_FULLY_OPAQUE = 255;        // Maximum alpha.

// How much room to pre-allocate in memory for the scene's triangles. The size
// will grow automatically if more is needed.
const uint TRIANGLE_CACHE_SIZE = 2048;
const uint UI_TRIANGLE_CACHE_SIZE = 512;

#endif
