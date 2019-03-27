#ifndef MUGGLE_UTILS_H_
#define MUGGLE_UTILS_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>

EXTERN_C_BEGIN

#define IS_POW_OF_2(x) (!(x & (x-1)))

uint64_t next_pow_of_2(uint64_t x);

EXTERN_C_END

#endif