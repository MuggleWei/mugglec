/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_CPU_CYCLE_H_
#define MUGGLE_C_CPU_CYCLE_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>

EXTERN_C_BEGIN

MUGGLE_C_EXPORT
uint64_t muggle_get_cpu_cycle();

EXTERN_C_END

#endif
