/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef __MUGGLE_SLEEP_H__
#define __MUGGLE_SLEEP_H__

#include "muggle/base/macro.h"

EXTERN_C_BEGIN

// sleep
#define MSleep(ms) SleepFunction(ms)

MUGGLE_BASE_EXPORT void SleepFunction(unsigned long ms);

EXTERN_C_END

#endif