/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_SLEEP_H_
#define MUGGLE_C_SLEEP_H_

#include "muggle/base_c/macro.h"

EXTERN_C_BEGIN

// sleep
#define MSleep(ms) SleepFunction(ms)

MUGGLE_BASE_C_EXPORT void SleepFunction(unsigned long ms);

EXTERN_C_END

#endif