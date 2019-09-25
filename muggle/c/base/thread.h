/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_THREAD_H_
#define MUGGLE_C_THREAD_H_

#include "muggle/c/base/macro.h"

#if MUGGLE_PLATFORM_WINDOWS
#include <windows.h>
typedef unsigned int muggle_thread_ret_t;
#else
#include <pthread.h>
typedef void* muggle_thread_ret_t;
#endif

EXTERN_C_BEGIN

typedef struct muggle_thread_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	HANDLE handle;
	unsigned int id;
#else
	pthread_t th;
#endif
}muggle_thread_t;

#if MUGGLE_PLATFORM_WINDOWS
typedef muggle_thread_ret_t __stdcall muggle_thread_routine(void *args);
#else
typedef muggle_thread_ret_t muggle_thread_routine(void *args);
#endif

MUGGLE_CC_EXPORT
int muggle_thread_create(muggle_thread_t *thread, muggle_thread_routine routine, void *args);

MUGGLE_CC_EXPORT
int muggle_thread_join(muggle_thread_t *thread);

MUGGLE_CC_EXPORT
int muggle_thread_detach(muggle_thread_t *thread);

EXTERN_C_END

#endif
