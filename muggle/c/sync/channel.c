/******************************************************************************
 *  @file         channel.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec channel
 *****************************************************************************/

#include "channel.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/sync/futex.h"

static void muggle_channel_lock_write(muggle_channel_t *chan)
{
	muggle_atomic_int expected = MUGGLE_CHANNEL_LOCK_STATUS_UNLOCK;
	while (!muggle_atomic_cmp_exch_weak(&chan->write_futex, &expected, MUGGLE_CHANNEL_LOCK_STATUS_LOCK, muggle_memory_order_acquire)
			&& expected != MUGGLE_CHANNEL_LOCK_STATUS_UNLOCK)
	{
		muggle_futex_wait(&chan->write_futex, expected, NULL);
		expected = MUGGLE_CHANNEL_LOCK_STATUS_UNLOCK;
	}
}

static void muggle_channel_unlock_write(muggle_channel_t *chan)
{
	muggle_atomic_store(&chan->write_futex, MUGGLE_CHANNEL_LOCK_STATUS_UNLOCK, muggle_memory_order_relaxed);
	muggle_futex_wake_one(&chan->write_futex);
}

/***************** write *****************/
static int muggle_channel_write_mutex(muggle_channel_t *chan, void *data)
{
	muggle_mutex_lock(&chan->write_mutex);

	if (chan->write_cursor + 1 == chan->read_cursor)
	{
		muggle_mutex_unlock(&chan->write_mutex);
		return MUGGLE_ERR_FULL;
	}

	chan->blocks[IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity)].data = data;
	muggle_atomic_store(&chan->write_cursor, chan->write_cursor + 1, muggle_memory_order_release);

	muggle_mutex_unlock(&chan->write_mutex);

	return MUGGLE_OK;
}
static int muggle_channel_write_single_writer(muggle_channel_t *chan, void *data)
{
	if (chan->write_cursor + 1 == chan->read_cursor)
	{
		return MUGGLE_ERR_FULL;
	}

	chan->blocks[IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity)].data = data;
	muggle_atomic_store(&chan->write_cursor, chan->write_cursor + 1, muggle_memory_order_release);

	return MUGGLE_OK;
}
static int muggle_channel_write_futex(muggle_channel_t *chan, void *data)
{
	muggle_channel_lock_write(chan);

	if (chan->write_cursor + 1 == chan->read_cursor)
	{
		muggle_channel_unlock_write(chan);
		return MUGGLE_ERR_FULL;
	}

	chan->blocks[IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity)].data = data;
	muggle_atomic_store(&chan->write_cursor, chan->write_cursor + 1, muggle_memory_order_release);

	muggle_channel_unlock_write(chan);

	return MUGGLE_OK;
}

/***************** wake *****************/
static void muggle_channel_wake_futex(struct muggle_channel *chan)
{
	muggle_futex_wake_one(&chan->write_cursor);
}
static void muggle_channel_wake_busy_loop(struct muggle_channel *chan)
{
	// do nothing
}

/***************** read *****************/
static void* muggle_channel_read_futex(struct muggle_channel *chan)
{
	muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(chan->read_cursor + 1, chan->capacity);
	muggle_atomic_int w_cursor;
	while (1)
	{
		w_cursor = muggle_atomic_load(&chan->write_cursor, muggle_memory_order_acquire);
		if (IDX_IN_POW_OF_2_RING(w_cursor, chan->capacity) != r_pos)
		{
			void *data = chan->blocks[r_pos].data;
			chan->read_cursor++;
			return data;
		}

		muggle_futex_wait(&chan->write_cursor, w_cursor, NULL);
	}

	return NULL;
}
static void* muggle_channel_read_busy_loop(struct muggle_channel *chan)
{
	muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(chan->read_cursor + 1, chan->capacity);
	while (1)
	{
		if (IDX_IN_POW_OF_2_RING(muggle_atomic_load(&chan->write_cursor, muggle_memory_order_acquire), chan->capacity) != r_pos)
		{
			void *data = chan->blocks[r_pos].data;
			chan->read_cursor++;
			return data;
		}

		muggle_thread_yield();
	}

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
	chan->read_cursor = capacity - 1;
	chan->write_futex = MUGGLE_CHANNEL_LOCK_STATUS_UNLOCK;

	int ret = muggle_mutex_init(&chan->write_mutex);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	chan->blocks = (muggle_channel_block_t*)malloc(sizeof(muggle_channel_block_t) * capacity);
	if (chan->blocks == NULL)
	{
		muggle_mutex_destroy(&chan->write_mutex);
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
	else if (chan->flags & MUGGLE_CHANNEL_FLAG_WRITE_FUTEX)
	{
		chan->fn_write = muggle_channel_write_futex;
	}
	else
	{
		chan->fn_write = muggle_channel_write_mutex;
	}

	if (chan->flags & MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP)
	{
		chan->fn_read = muggle_channel_read_busy_loop;
		chan->fn_wake = muggle_channel_wake_busy_loop;
	}
	else
	{
		chan->fn_read = muggle_channel_read_futex;
		chan->fn_wake = muggle_channel_wake_futex;
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

	muggle_mutex_destroy(&chan->write_mutex);
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
