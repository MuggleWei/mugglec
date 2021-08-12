/******************************************************************************
 *  @file         channel.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec channel
 *
 * Passing data between threads, user must gurantee only one reader use channel at the same time
 * When channel full, write will failed and return enum MUGGLE_ERR_*
 * When channel empty, read will block until data write into channel
 *
 * muggle_channel_t and muggle_ring_buffer_t with only one reader are very similar, but most important different is
 * when ringbuffer full, last message will be pushed in and lost all old message
 * when channel full, write will return failed
 *****************************************************************************/

#ifndef MUGGLE_C_CHANNEL_H_
#define MUGGLE_C_CHANNEL_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"
#include "muggle/c/sync/mutex.h"
#include "muggle/c/sync/condition_variable.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_CHANNEL_FLAG_WRITE_MUTEX    = 0x00, //!< write lock use mutex
	MUGGLE_CHANNEL_FLAG_READ_WAIT      = 0x00, //!< reader wait

	MUGGLE_CHANNEL_FLAG_SINGLE_WRITER  = 0x01, //!< user guarantee only one writer use this channel
	MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP = 0x02, //!< reader busy loop until read message from channel
	MUGGLE_CHANNEL_FLAG_WRITE_FUTEX    = 0x04, //!< write lock use futex if be support
};

enum
{
	MUGGLE_CHANNEL_LOCK_STATUS_UNLOCK,
	MUGGLE_CHANNEL_LOCK_STATUS_LOCK,
};

struct muggle_channel;
typedef int (*fn_muggle_channel_write)(struct muggle_channel *chan, void *data);
typedef void* (*fn_muggle_channel_read)(struct muggle_channel *chan);
typedef void (*fn_muggle_channel_wake)(struct muggle_channel *chan);

#if MUGGLE_SUPPORT_FUTEX

/**
 * @brief channel node block
 */
typedef struct muggle_channel_block
{
	MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	void *data;
}muggle_channel_block_t;

/**
 * @brief channel
 */
typedef struct muggle_channel
{
	MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	muggle_atomic_int       capacity;
	int                     flags;
	fn_muggle_channel_write fn_write;
	fn_muggle_channel_read  fn_read;
	fn_muggle_channel_wake  fn_wake;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(1);
	muggle_atomic_int write_cursor;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(2);
	muggle_atomic_int read_cursor;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(3);
	muggle_atomic_int write_futex;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(4);
	muggle_mutex_t write_mutex;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(5);
	muggle_channel_block_t *blocks;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(6);
}muggle_channel_t;

#else

/**
 * @brief channel node block
 */
typedef struct muggle_channel_block
{
	void *data;
}muggle_channel_block_t;

typedef struct muggle_channel
{
	muggle_atomic_int       capacity;
	int                     flags;
	fn_muggle_channel_write fn_write;
	fn_muggle_channel_read  fn_read;
	fn_muggle_channel_wake  fn_wake;

	muggle_atomic_int write_cursor;
	muggle_atomic_int read_cursor;
	muggle_mutex_t write_mutex;
	muggle_mutex_t read_mutex;
	muggle_condition_variable_t read_cv;
	muggle_channel_block_t *blocks;
}muggle_channel_t;

#endif

/**
 * @brief init muggle_channel_t
 *
 * @param chan      pointer to muggle_channel_t
 * @param capacity  capacity of channel
 * @param flags     bitwise or of MUGGLE_CHANNEL_FLAG_*
 *
 * @return
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_channel_init(muggle_channel_t *chan, muggle_atomic_int capacity, int flags);

/**
 * @brief destroy muggle_channel_t
 *
 * @param chan  pointer to muggle_channel_t
 */
MUGGLE_C_EXPORT
void muggle_channel_destroy(muggle_channel_t *chan);

/**
 * @brief write data into channel
 *
 * @param chan  pointer to muggle_channel_t
 * @param data  data pointer
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_channel_write(muggle_channel_t *chan, void *data);

/**
 * @brief read data from channel
 *
 * @param chan  pointer to muggle_channel_t
 *
 * @return data pointer
 */
MUGGLE_C_EXPORT
void* muggle_channel_read(muggle_channel_t *chan);

EXTERN_C_END

#endif
