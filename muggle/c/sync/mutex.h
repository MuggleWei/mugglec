/******************************************************************************
 *  @file         mutex.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec mutex
 *****************************************************************************/

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

/**
 * @brief initialize mutex
 *
 * @param mutex mutex pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_mutex_init(muggle_mutex_t *mutex);

/**
 * @brief destroy mutex
 *
 * @param mutex mutex pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_mutex_destroy(muggle_mutex_t *mutex);

/**
 * @brief lock mutex
 *
 * @param mutex mutex pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_mutex_lock(muggle_mutex_t *mutex);

/**
 * @brief try lock mutex
 *
 * @param mutex mutex pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_mutex_trylock(muggle_mutex_t *mutex);

/**
 * @brief unlock mutex
 *
 * @param mutex mutex pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_mutex_unlock(muggle_mutex_t *mutex);

EXTERN_C_END

#endif
