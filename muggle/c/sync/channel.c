/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "channel.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/sync/futex.h"

// writer functions
static int muggle_channel_write_default(struct muggle_channel *chan, void *data)
{
	// move next
	muggle_atomic_int expected = chan->next_write_cursor;
	muggle_atomic_int next = expected;
	if (next == chan->read_cursor)
	{
		return MUGGLE_ERR_FULL;
	}

	while (!muggle_atomic_cmp_exch_weak(&chan->next_write_cursor, &expected, next+1, muggle_memory_order_relaxed)
			&& expected != next)
	{
		muggle_thread_yield();
		next = expected;
		if (next == chan->read_cursor)
		{
			return MUGGLE_ERR_FULL;
		}
	}

	// assignment
	chan->blocks[IDX_IN_POW_OF_2_RING(next, chan->capacity)].data = data;

	// move write cursor
	muggle_atomic_int w_pos = next;
	while (!muggle_atomic_cmp_exch_weak(&chan->write_cursor, &w_pos, next + 1, muggle_memory_order_release)
			&& w_pos != next)
	{
		muggle_thread_yield();
		w_pos = next;
	}

	return MUGGLE_OK;
}
static int muggle_channel_write_single_writer(struct muggle_channel *chan, void *data)
{
	if (chan->write_cursor == chan->read_cursor)
	{
		return MUGGLE_ERR_FULL;
	}

	// assignment
	muggle_channel_block_t *block = &chan->blocks[IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity)];
	block->data = data;

	// move cursor
	muggle_atomic_fetch_add(&chan->write_cursor, 1, muggle_memory_order_release);

	return MUGGLE_OK;
}

// weak functions
static void muggle_channel_wake_default(struct muggle_channel *chan)
{
	muggle_futex_wake_one(&chan->write_cursor);
}
static void muggle_channel_wake_busy_loop(struct muggle_channel *chan)
{
	// do nothing
}

// read functions
static void* muggle_channel_read_default(struct muggle_channel *chan)
{
	muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(chan->read_cursor + 1, chan->capacity);
	muggle_atomic_int w_cursor;
	do {
		w_cursor = muggle_atomic_load(&chan->write_cursor, muggle_memory_order_acquire);
		if (IDX_IN_POW_OF_2_RING(w_cursor, chan->capacity) != r_pos)
		{
			void *data = chan->blocks[r_pos].data;
			chan->read_cursor++;
			return data;
		}

		muggle_futex_wait(&chan->write_cursor, w_cursor, NULL);
	} while(1);

	return NULL;
}
static void* muggle_channel_read_busy_loop(struct muggle_channel *chan)
{
	muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(chan->read_cursor + 1, chan->capacity);
	muggle_atomic_int w_cursor;
	do {
		w_cursor = muggle_atomic_load(&chan->write_cursor, muggle_memory_order_acquire);
		if (IDX_IN_POW_OF_2_RING(w_cursor, chan->capacity) != r_pos)
		{
			void *data = chan->blocks[r_pos].data;
			chan->read_cursor++;
			return data;
		}

		muggle_thread_yield();
	} while (1);

	return NULL;
}

int muggle_channel_init(muggle_channel_t *chan, muggle_atomic_int capacity, int flags)
{
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	capacity = (muggle_atomic_int)next_pow_of_2((uint64_t)capacity);
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	chan->capacity = capacity;
	chan->flags = flags;
	chan->write_cursor = 0;
	chan->next_write_cursor = 0;
	chan->read_cursor = capacity - 1;
	chan->blocks = (muggle_channel_block_t*)malloc(sizeof(muggle_channel_block_t) * capacity);
	if (chan->blocks == NULL)
	{
		return MUGGLE_ERR_MEM_ALLOC;
	}

	for (muggle_atomic_int i = 0; i < capacity; i++)
	{
		memset(&chan->blocks[i], 0, sizeof(muggle_channel_block_t));
	}

	if (chan->flags & MUGGLE_CHANNEL_FLAG_SINGLE_WRITER)
	{
		chan->fn_write = muggle_channel_write_single_writer;
	}
	else
	{
		chan->fn_write = muggle_channel_write_default;
	}

	if (chan->flags & MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP)
	{
		chan->fn_read = muggle_channel_read_busy_loop;
		chan->fn_wake = muggle_channel_wake_busy_loop;
	}
	else
	{
		chan->fn_read = muggle_channel_read_default;
		chan->fn_wake = muggle_channel_wake_default;
	}

	return MUGGLE_OK;
}

void muggle_channel_destroy(muggle_channel_t *chan)
{
	if (chan->blocks)
	{
		free(chan->blocks);
		chan->blocks = NULL;
	}
}

int muggle_channel_write(muggle_channel_t *chan, void *data)
{
	int ret = chan->fn_write(chan, data);
	if (ret == 0)
	{
		chan->fn_wake(chan);
	}
	return ret;
}

void* muggle_channel_read(muggle_channel_t *chan)
{
	return chan->fn_read(chan);
}
