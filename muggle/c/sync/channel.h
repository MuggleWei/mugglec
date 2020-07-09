/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_CHANNEL_H_
#define MUGGLE_C_CHANNEL_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"

EXTERN_C_BEGIN

/*
 * channel
 * Passing data between threads, user must gurantee only one reader use channel at the same time
 * When channel full, write will failed and return enum MUGGLE_ERR_*
 * When channel empty, read will block until data write into channel
 *
 * muggle_channel_t and muggle_ring_buffer_t with only one reader are very similar, but most important different is
 * when ringbuffer full, last message will cover oldest message, the oldest message will lost
 * when channel full, write will return failed
 * */

enum
{
	MUGGLE_CHANNEL_FLAG_SINGLE_WRITER  = 0x01, // user guarantee only one writer use this channel
	MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP = 0x02, // reader busy loop until read message from channel
};

enum
{
	MUGGLE_CHANNEL_LOCK_STATUS_UNLOCK,
	MUGGLE_CHANNEL_LOCK_STATUS_LOCK,
};

typedef struct muggle_channel_block
{
	MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	void *data;
}muggle_channel_block_t;

struct muggle_channel;
typedef int (*fn_muggle_channel_write)(struct muggle_channel *chan, void *data);
typedef void* (*fn_muggle_channel_read)(struct muggle_channel *chan);
typedef void (*fn_muggle_channel_wake)(struct muggle_channel *chan);

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
	muggle_atomic_int write_lock;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(4);
	muggle_channel_block_t *blocks;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(5);
}muggle_channel_t;

/*
 * init muggle_channel_t
 * @chan: pointer to muggle_channel_t
 * @capacity: capacity of channel
 * @flags: bitwise or of MUGGLE_CHANNEL_FLAG_*
 * RETURN: 0 - success, otherwise failed and return MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_channel_init(muggle_channel_t *chan, muggle_atomic_int capacity, int flags);

/*
 * destroy muggle_channel_t
 * */
MUGGLE_CC_EXPORT
void muggle_channel_destroy(muggle_channel_t *chan);

/*
 * write data into channel
 * */
MUGGLE_CC_EXPORT
int muggle_channel_write(muggle_channel_t *chan, void *data);

/*
 * read data from channel
 * */
MUGGLE_CC_EXPORT
void* muggle_channel_read(muggle_channel_t *chan);

EXTERN_C_END

#endif
