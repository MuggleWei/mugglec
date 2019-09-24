/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_FUTEX_H_
#define MUGGLE_C_FUTEX_H_

#include "muggle/c/base/macro.h"
#include <time.h>
#include "muggle/c/base/atomic.h"

EXTERN_C_BEGIN

MUGGLE_CC_EXPORT
int muggle_futex_wait(muggle_atomic_int *futex_addr, muggle_atomic_int val, const struct timespec *timeout);

MUGGLE_CC_EXPORT
void muggle_futex_wake_one(muggle_atomic_int *futex_addr);

MUGGLE_CC_EXPORT
void muggle_futex_wake_all(muggle_atomic_int *futex_addr);

EXTERN_C_END

#endif
