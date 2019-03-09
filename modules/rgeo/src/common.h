#ifndef COMMON_H_
#define COMMON_H_

#include "types.h"

#define NUM_ELEMENTS(array) (int)(sizeof((array)) / sizeof((array)[0]))

#define k_assert(condition, message) if (!(condition))\
                                     {\
                                         ERROR((message));\
                                         assert((condition) && (message));\
                                     }

#define DEBUG(args) (printf("[debug] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))
#define ERROR(args) (printf("[error] {%s:%i} ", __FILE__, __LINE__), printf args, printf("\n"), fflush(stdout))

#endif
