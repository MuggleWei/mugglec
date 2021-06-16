/******************************************************************************
 *  @file         thread.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        function prototypes for mugglec thread
 *****************************************************************************/

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

/**
 * @brief starts a new thread in the calling process
 *
 * @param thread   pointer to thread handler
 * @param routine  thread callback
 * @param args     arguments passing to thread function
 *
 * @return
 *     0 - success start thread
 *     otherwise return errno in mugglec/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_thread_create(muggle_thread_t *thread, muggle_thread_routine routine, void *args);

/**
 * @brief waits for the thread specified by thread to terminate
 *
 * @param thread  pointer to thread handler
 *
 * @return
 *     0 - success start thread
 *     otherwise return errno in mugglec/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_thread_join(muggle_thread_t *thread);

/**
 * @brief marks the thread identified by thread as detached
 *
 * @param thread  pointer to thread handler
 *
 * @return
 *     0 - success start thread
 *     otherwise return errno in mugglec/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_thread_detach(muggle_thread_t *thread);

/**
 * @brief get current thread id
 *
 * @return current thread id
 */
MUGGLE_C_EXPORT
muggle_thread_id muggle_thread_current_id();

/**
 * @brief return the number of concurrent threads supported by the implementation
 *
 * @return number of concurrent threads supported
 */
MUGGLE_C_EXPORT
int muggle_thread_hardware_concurrency();

/**
 * @brief calling thread to yield execution and relinquish the CPU
 */
MUGGLE_C_EXPORT
void muggle_thread_yield();

EXTERN_C_END

#endif
