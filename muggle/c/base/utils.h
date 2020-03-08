/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_UTILS_H_
#define MUGGLE_UTILS_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>

EXTERN_C_BEGIN

#define IS_POW_OF_2(x)             (!((x)&((x)-1)))
#define ROUND_UP_POW_OF_2_MUL(x,n) (((x)+(n)-1)&~((n)-1))

MUGGLE_CC_EXPORT
uint64_t next_pow_of_2(uint64_t x);

EXTERN_C_END

#endif
