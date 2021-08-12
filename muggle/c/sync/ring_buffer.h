/******************************************************************************
 *  @file         ring_buffer.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec ring buffer
 *****************************************************************************/

#ifndef MUGGLE_C_RING_BUFFER_H_
#define MUGGLE_C_RING_BUFFER_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"
#include "muggle/c/sync/mutex.h"
#include "muggle/c/sync/condition_variable.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK      = 0x00, //!< default, write with write lock
	MUGGLE_RING_BUFFER_FLAG_READ_ALL        = 0x00, //!< default, every reader get all message from all writer
	MUGGLE_RING_BUFFER_FLAG_READ_WAIT       = 0x00, //!< default, if no message in ring, reader wait

	MUGGLE_RING_BUFFER_FLAG_SINGLE_WRITER   = 0x01, //!< user guarantee only one writer use this ring_buffer
	MUGGLE_RING_BUFFER_FLAG_SINGLE_READER   = 0x02, //!< user guarantee only one reader use this ring_buffer
	MUGGLE_RING_BUFFER_FLAG_WRITE_BUSY_LOOP = 0x04, //!< every writer busy loop until write message into ring
	MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP  = 0x08, //!< reader busy loop until read message from ring
	MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE   = 0x10, //!< every message will only be read once
};

#if MUGGLE_SUPPORT_FUTEX

typedef struct muggle_ring_buffer_tag
{
	MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	muggle_atomic_int capacity;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(1);
	int flag;
	int write_mode;
	int read_mode;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(2);
	muggle_atomic_int next;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(3);
	muggle_atomic_int cursor;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(4);
	muggle_atomic_int read_cursor; // for MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE
	MUGGLE_STRUCT_CACHE_LINE_PADDING(5);
	muggle_mutex_t write_mutex;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(6);
	muggle_mutex_t read_mutex;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(7);
	muggle_condition_variable_t read_cv;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(8);
	void **datas;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(9);
}muggle_ring_buffer_t;

#else

typedef struct muggle_ring_buffer_tag
{
	muggle_atomic_int capacity;
	int flag;
	int write_mode;
	int read_mode;
	muggle_atomic_int next;
	muggle_atomic_int cursor;
	muggle_atomic_int read_cursor; // for MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE
	muggle_mutex_t mtx;
	muggle_condition_variable_t cv;
	void **datas;
}muggle_ring_buffer_t;

#endif  /* #if MUGGLE_SUPPORT_FUTEX */

/**
 * @brief initialize ring buffer
 *
 * @param r         ring buffer pointer
 * @param capacity  initialize capacity for ring buffer
 * @param flag      bit OR operationg of MUGGLE_RING_BUFFER_FLAG_*
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_ring_buffer_init(muggle_ring_buffer_t *r, muggle_atomic_int capacity, int flag);

/**
 * @brief destroy ring buffer
 *
 * @param r  ring buffer pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_ring_buffer_destroy(muggle_ring_buffer_t *r);

/**
 * @brief write data into ring buffer
 *
 * @param r     ring buffer pointer
 * @param data  data pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_ring_buffer_write(muggle_ring_buffer_t *r, void *data);

/**
 * @brief read data from ring buffer
 *
 * @param r     ring buffer pointer
 * @param idx   index of data
 *
 * @return data pointer
 */
MUGGLE_C_EXPORT
void* muggle_ring_buffer_read(muggle_ring_buffer_t *r, muggle_atomic_int idx);

EXTERN_C_END

#endif
