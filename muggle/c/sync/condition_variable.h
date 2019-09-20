/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_CONDITION_VARIABLE_H_
#define MUGGLE_C_CONDITION_VARIABLE_H_

#include "muggle/c/base/macro.h"

#if MUGGLE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#include <limits.h>	// for ULONG_MAX (MUGGLE_WAIT_INFINITE expand)
#endif
#include <time.h>
#include "muggle/c/sync/mutex.h"

EXTERN_C_BEGIN

typedef struct muggle_condition_variable_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	CONDITION_VARIABLE cond_var;
#else
	pthread_cond_t cond_var;
#endif
}muggle_condition_variable_t;

MUGGLE_CC_EXPORT
int muggle_condition_variable_init(muggle_condition_variable_t *cv);

MUGGLE_CC_EXPORT
int muggle_condition_variable_destroy(muggle_condition_variable_t *cv);

MUGGLE_CC_EXPORT
int muggle_condition_variable_wait(muggle_condition_variable_t *cv, muggle_mutex_t *mutex, const struct timespec *timeout);

MUGGLE_CC_EXPORT
int muggle_condition_variable_notify_one(muggle_condition_variable_t *cv);

MUGGLE_CC_EXPORT
int muggle_condition_variable_notify_all(muggle_condition_variable_t *cv);

EXTERN_C_END

#endif
