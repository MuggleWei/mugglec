/******************************************************************************
 *  @file         condition_variable.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec condition variable
 *****************************************************************************/

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

/**
 * @brief condition variable
 */
typedef struct muggle_condition_variable_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	CONDITION_VARIABLE cond_var;
#else
	pthread_cond_t cond_var;
#endif
}muggle_condition_variable_t;

/**
 * @brief initialize condition variable
 *
 * @param cv condition variable pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_condition_variable_init(muggle_condition_variable_t *cv);

/**
 * @brief destroy condition variable
 *
 * @param cv condition variable pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_condition_variable_destroy(muggle_condition_variable_t *cv);

/**
 * @brief current thread block until condition variable is notified
 *
 * @param cv       condition variable pointer
 * @param mutex    mutex pointer
 * @param timeout  timeout
 *
 * @return 
 */
MUGGLE_C_EXPORT
int muggle_condition_variable_wait(muggle_condition_variable_t *cv, muggle_mutex_t *mutex, const struct timespec *timeout);

/**
 * @brief notify one waiting thread
 *
 * @param cv  condition variable pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_condition_variable_notify_one(muggle_condition_variable_t *cv);

/**
 * @brief notify all waiting thread
 *
 * @param cv  condition variable pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_condition_variable_notify_all(muggle_condition_variable_t *cv);

EXTERN_C_END

#endif
