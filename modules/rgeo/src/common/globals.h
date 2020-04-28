#ifndef COMMON_H_
#define COMMON_H_

#include <assert.h>
#include <stdio.h>

// The minimum RallySportED loader version required for tracks made with this
// version of the track editor.
#define REQUIRED_LOADER_VER 5

#define NUM_ELEMENTS(array) (int)(sizeof((array)) / sizeof((array)[0]))

#define k_assert(condition, message) if (!(condition))\
                                     {\
                                         ERROR(((message)));\
                                         assert((condition) && (message));\
                                     }

// For assertions that aren't crucial and/or carry a considerable run-time
// performance penalty, such that performing them may be optionally disabled.
#define k_optional_assert k_assert

#define ERROR(args) (printf("[ERROR] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))
#define DEBUG(args) (printf("[DEBUG] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))

#endif
