/******************************************************************************
 *  @file         double_buffer.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec double buffer
 *
 * double-buffer just for multiple writer one reader, don't use it
 * with multiple reader
 *****************************************************************************/

#ifndef MUGGLE_C_DOUBLE_BUFFER_H_
#define MUGGLE_C_DOUBLE_BUFFER_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/sync/mutex.h"
#include "muggle/c/sync/condition_variable.h"

EXTERN_C_BEGIN

/**
 * @brief buffer in double buffer
 */
typedef struct muggle_single_buffer_tag
{
	void **datas;
	int cnt;
}muggle_single_buffer_t;

/**
 * @brief double buffer
 */
typedef struct muggle_double_buffer_tag
{
	muggle_single_buffer_t buf[2];
	muggle_single_buffer_t *front;
	muggle_single_buffer_t *back;
	int capacity;
	int non_blocking;
	muggle_mutex_t mutex;
	muggle_condition_variable_t cv_not_empty;
	muggle_condition_variable_t cv_not_full;
}muggle_double_buffer_t;

/**
 * @brief initialize double buffer
 *
 * @param buf           double buffer pointer
 * @param capacity      initialized capacity for double buffer
 * @param non_blocking  double buffer is blocking
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_double_buffer_init(muggle_double_buffer_t *buf, int capacity, int non_blocking);

/**
 * @brief destroy double buffer
 *
 * @param buf  double buffer pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_double_buffer_destroy(muggle_double_buffer_t *buf);

/**
 * @brief write data into double buffer
 *
 * @param buf   double buffer pointer
 * @param data  data pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_double_buffer_write(muggle_double_buffer_t *buf, void *data);

/**
 * @brief swap buffer and return buffer for read
 *
 * @param buf   double buffer pointer
 *
 * @return buffer in double buffer that need to read
 */
MUGGLE_C_EXPORT
muggle_single_buffer_t* muggle_double_buffer_read(muggle_double_buffer_t *buf);

EXTERN_C_END

#endif
