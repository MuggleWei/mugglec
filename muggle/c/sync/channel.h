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

/**
 * @brief channel write flags
 */
enum
{
	MUGGLE_CHANNEL_FLAG_WRITE_MUTEX   = 0, //!< write lock use mutex
	MUGGLE_CHANNEL_FLAG_WRITE_FUTEX   = 1, //!< write lock use futex if be supported
	MUGGLE_CHANNEL_FLAG_WRITE_SPIN    = 2, //!< write lock use spinlock
	MUGGLE_CHANNEL_FLAG_WRITE_SINGLE  = 3, //!< user guarantee only one writer use this channel

	MUGGLE_CHANNEL_FLAG_SINGLE_WRITER = MUGGLE_CHANNEL_FLAG_WRITE_SINGLE,
};

/**
 * @brief channel read flags
 */
enum
{
	MUGGLE_CHANNEL_FLAG_READ_MUTEX = 0 << 4, //!< reader with mutex with
	MUGGLE_CHANNEL_FLAG_READ_FUTEX = 1 << 4, //!< reader with futex wait if supported
	MUGGLE_CHANNEL_FLAG_READ_BUSY  = 2 << 4, //!< reader busy loop until read message from channel

	MUGGLE_CHANNEL_FLAG_READ_WAIT      = MUGGLE_CHANNEL_FLAG_READ_FUTEX,
	MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP = MUGGLE_CHANNEL_FLAG_READ_BUSY,
};

/**
 * @brief channel flags mask
 */
enum
{
	MUGGLE_CHANNEL_FLAG_MASK_W = 0x0f,
	MUGGLE_CHANNEL_FLAG_MASK_R = 0xf0
};

struct muggle_channel;

typedef void (*fn_muggle_channel_lock)(struct muggle_channel *chan);
typedef int (*fn_muggle_channel_write)(struct muggle_channel *chan, void *data);
typedef void (*fn_muggle_channel_unlock)(struct muggle_channel *chan);
typedef void (*fn_muggle_channel_wake)(struct muggle_channel *chan);
typedef void* (*fn_muggle_channel_read)(struct muggle_channel *chan);

/**
 * @brief channel node block
 */
typedef struct muggle_channel_block
{
	union {
		void *data;
		MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	};
}muggle_channel_block_t;

/**
 * @brief channel
 */
typedef struct muggle_channel
{
	muggle_atomic_int       capacity;   //!< capacity of channel
	int                     flags;      //!< channel flags
	int                     init_flags; //!< initialized flags
	fn_muggle_channel_lock  fn_lock;    //!< write lock function
	fn_muggle_channel_lock  fn_unlock;  //!< write unlock function
	fn_muggle_channel_write fn_write;   //!< write function
	fn_muggle_channel_wake  fn_wake;    //!< wake function
	fn_muggle_channel_read  fn_read;    //!< read function
	union {
		muggle_atomic_int write_cursor;
		MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	};
	union {
		muggle_atomic_int read_cursor;
		MUGGLE_STRUCT_CACHE_LINE_PADDING(1);
	};
	union {
		// write lock
		muggle_atomic_int write_futex;
		muggle_mutex_t write_mutex;
		muggle_atomic_int write_spinlock;
		MUGGLE_STRUCT_CACHE_LINE_PADDING(2);
	};
	// read lock
	muggle_mutex_t read_mutex;
	muggle_condition_variable_t read_cv;
	muggle_channel_block_t *blocks;
}muggle_channel_t;

/**
 * @brief init muggle_channel_t
 *
 * @param chan      pointer to muggle_channel_t
 * @param capacity  capacity of channel
 * @param flags     bitwise or MUGGLE_CHANNEL_FLAG_WRITE_* and MUGGLE_CHANNEL_FLAG_READ_*
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
