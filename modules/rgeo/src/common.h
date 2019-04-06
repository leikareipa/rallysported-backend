#ifndef COMMON_H_
#define COMMON_H_

#include <assert.h>
#include <stdio.h>
#include "types.h"

/* The minimum RallySportED loader version required for tracks made with this
 * version of the track editor.*/
#define REQUIRED_LOADER_VER 3

#define NUM_ELEMENTS(array) (int)(sizeof((array)) / sizeof((array)[0]))

#define k_assert(condition, message) if (!(condition))\
                                     {\
                                         ERROR((message));\
                                         assert((condition) && (message));\
                                     }

#define ERROR(args) (printf("[error] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))
#define DEBUG(args) (printf("[debug] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))

#endif
