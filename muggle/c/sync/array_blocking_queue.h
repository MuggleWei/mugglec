/******************************************************************************
 *  @file         array_blocking_queue.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec array blocking queue
 *****************************************************************************/

#ifndef MUGGLE_C_ARRAY_BLOCKING_QUEUE_H_
#define MUGGLE_C_ARRAY_BLOCKING_QUEUE_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/sync/mutex.h"
#include "muggle/c/sync/condition_variable.h"

EXTERN_C_BEGIN

/**
 * @brief array blocking queue
 */
typedef struct muggle_array_blocking_queue_tag
{
	void **datas;
	int capacity;
	int take_idx;
	int put_idx;
	int cnt;
	muggle_mutex_t mutex;
	muggle_condition_variable_t cv_not_empty;
	muggle_condition_variable_t cv_not_full;
}muggle_array_blocking_queue_t;

/**
 * @brief initialize array blocking queue
 *
 * @param queue     array blocking queue pointer
 * @param capacity  initialize capacity for queue
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_array_blocking_queue_init(muggle_array_blocking_queue_t *queue, int capacity);

/**
 * @brief destroy array blocking queue
 *
 * @param queue   array blocking queue pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_array_blocking_queue_destroy(muggle_array_blocking_queue_t *queue);

/**
 * @brief put data into queue
 *
 * @param queue   array blocking queue pointer
 * @param data    data pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_array_blocking_queue_put(muggle_array_blocking_queue_t *queue, void *data);

/**
 * @brief take data from queue
 *
 * @param queue   array blocking queue pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
void* muggle_array_blocking_queue_take(muggle_array_blocking_queue_t *queue);

EXTERN_C_END

#endif
