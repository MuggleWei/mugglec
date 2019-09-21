/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_H_
#define MUGGLE_C_H_

// base c header
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>

// muggle c base header
#include "muggle/c/base/macro.h"
#include "muggle/c/base/err.h"
#include "muggle/c/base/log.h"
#include "muggle/c/base/file.h"
#include "muggle/c/base/str.h"
#include "muggle/c/base/sleep.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/base/atomic.h"
#include "muggle/c/base/semaphore.h"
#include "muggle/c/base/utils.h"
#if MUGGLE_PLATFORM_WINDOWS
#include "muggle/c/base/win_gettimeofday.h"
#endif

// muggle c memory pool
#include "muggle/c/memory_pool/memory_pool.h"

// muggle c time
#include "muggle/c/time/delta_time.h"

// os
#include "muggle/c/os/dl.h"

// sync
#include "muggle/c/sync/mutex.h"
#include "muggle/c/sync/condition_variable.h"
#include "muggle/c/sync/futex.h"
#include "muggle/c/sync/ringbuffer.h"

#endif
