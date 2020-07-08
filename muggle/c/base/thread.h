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

#if MUGGLE_PLATFORM_WINDOWS
typedef DWORD muggle_thread_id;
#else
typedef pthread_t muggle_thread_id;
#endif

/*
 * starts a new thread in the calling process
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_thread_create(muggle_thread_t *thread, muggle_thread_routine routine, void *args);

/*
 * waits for the thread specified by thread to terminate
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_thread_join(muggle_thread_t *thread);

/*
 * marks the thread identified by thread as detached
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_thread_detach(muggle_thread_t *thread);

/*
 * get current thread id
 * */
MUGGLE_CC_EXPORT
muggle_thread_id muggle_thread_current_id();

/*
 * the same as c++11 std::thread::hardware_concurrency
 * */
MUGGLE_CC_EXPORT
int muggle_thread_hardware_concurrency();

/*
 * calling thread to yield execution and relinquish the CPU
 * */
MUGGLE_CC_EXPORT
void muggle_thread_yield();

EXTERN_C_END

#endif
