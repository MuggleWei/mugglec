/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_MUTEX_H_
#define MUGGLE_C_MUTEX_H_

#include "muggle/c/base/macro.h"

#if MUGGLE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif

EXTERN_C_BEGIN

typedef struct muggle_mutex_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	CRITICAL_SECTION cs;
#else
	pthread_mutex_t mtx;
#endif
}muggle_mutex_t;

MUGGLE_C_EXPORT
int muggle_mutex_init(muggle_mutex_t *mutex);

MUGGLE_C_EXPORT
int muggle_mutex_destroy(muggle_mutex_t *mutex);

MUGGLE_C_EXPORT
int muggle_mutex_lock(muggle_mutex_t *mutex);

MUGGLE_C_EXPORT
int muggle_mutex_trylock(muggle_mutex_t *mutex);

MUGGLE_C_EXPORT
int muggle_mutex_unlock(muggle_mutex_t *mutex);

EXTERN_C_END

#endif
