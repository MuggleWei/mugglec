#include "channel.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/base/thread.h"

#ifndef MUGGLE_SUPPORT_FUTEX

/***************** write *****************/
static int muggle_channel_mutex_write_mutex_read(muggle_channel_t *chan, void *data)
{
	muggle_mutex_lock(&chan->write_mutex);

	if (chan->write_cursor + 1 == chan->read_cursor)
	{
		muggle_mutex_unlock(&chan->write_mutex);
		return MUGGLE_ERR_FULL;
	}

	muggle_atomic_int w_cursor = IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity);

	muggle_mutex_lock(&chan->read_mutex);
	chan->blocks[w_cursor].data = data;
	chan->write_cursor++;
	muggle_mutex_unlock(&chan->read_mutex);

	muggle_mutex_unlock(&chan->write_mutex);

	return MUGGLE_OK;
}

static int muggle_channel_single_write_mutex_read(muggle_channel_t *chan, void *data)
{
	if (chan->write_cursor + 1 == chan->read_cursor)
	{
		muggle_mutex_unlock(&chan->write_mutex);
		return MUGGLE_ERR_FULL;
	}

	muggle_atomic_int w_cursor = IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity);

	muggle_mutex_lock(&chan->read_mutex);
	chan->blocks[w_cursor].data = data;
	chan->write_cursor++;
	muggle_mutex_unlock(&chan->read_mutex);

	return MUGGLE_OK;
}

static int muggle_channel_mutex_write_busy_read(muggle_channel_t *chan, void *data)
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

static int muggle_channel_single_write_busy_read(muggle_channel_t *chan, void *data)
{
	if (chan->write_cursor + 1 == chan->read_cursor)
	{
		muggle_mutex_unlock(&chan->write_mutex);
		return MUGGLE_ERR_FULL;
	}

	chan->blocks[IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity)].data = data;
	muggle_atomic_store(&chan->write_cursor, chan->write_cursor + 1, muggle_memory_order_release);

	return MUGGLE_OK;
}

/***************** wake *****************/
static void muggle_channel_wake_mutex(muggle_channel_t *chan)
{
	muggle_condition_variable_notify_one(&chan->read_cv);
}
static void muggle_channel_wake_busy_loop(muggle_channel_t *chan)
{
	// do nothing
}

/***************** read *****************/
static void* muggle_channel_read_mutex(muggle_channel_t *chan)
{
	muggle_mutex_lock(&chan->read_mutex);

	while (1)
	{
		muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(chan->read_cursor + 1, chan->capacity);
		muggle_atomic_int w_pos = IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity);
		if (r_pos != w_pos)
		{
			void *data = chan->blocks[r_pos].data;
			chan->read_cursor++;
			muggle_mutex_unlock(&chan->read_mutex);
			return data;
		}

		muggle_condition_variable_wait(&chan->read_cv, &chan->read_mutex, NULL);
	}
}
static void* muggle_channel_read_busy_loop(muggle_channel_t *chan)
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

	int ret = muggle_mutex_init(&chan->write_mutex);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	ret = muggle_mutex_init(&chan->read_mutex);
	if (ret != MUGGLE_OK)
	{
		muggle_mutex_destroy(&chan->write_mutex);
		return ret;
	}

	ret = muggle_condition_variable_init(&chan->read_cv);
	if (ret != MUGGLE_OK)
	{
		muggle_mutex_destroy(&chan->write_mutex);
		muggle_mutex_destroy(&chan->read_mutex);
		return ret;
	}

	chan->blocks = (muggle_channel_block_t*)malloc(sizeof(muggle_channel_block_t) * capacity);
	if (chan->blocks == NULL)
	{
		muggle_mutex_destroy(&chan->write_mutex);
		muggle_mutex_destroy(&chan->read_mutex);
		muggle_condition_variable_destroy(&chan->read_cv);
		return MUGGLE_ERR_MEM_ALLOC;
	}

	for (muggle_atomic_int i = 0; i < capacity; i++)
	{
		memset(&chan->blocks[i], 0, sizeof(muggle_channel_block_t));
	}

	if (chan->flags & MUGGLE_CHANNEL_FLAG_SINGLE_WRITER)
	{
		if (chan->flags & MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP)
		{
			chan->fn_write = muggle_channel_single_write_busy_read;
			chan->fn_wake = muggle_channel_wake_busy_loop;
			chan->fn_read = muggle_channel_read_busy_loop;
		}
		else
		{
			chan->fn_write = muggle_channel_single_write_mutex_read;
			chan->fn_wake = muggle_channel_wake_mutex;
			chan->fn_read = muggle_channel_read_mutex;
		}
	}
	else
	{
		if (chan->flags & MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP)
		{
			chan->fn_write = muggle_channel_mutex_write_busy_read;
			chan->fn_wake = muggle_channel_wake_busy_loop;
			chan->fn_read = muggle_channel_read_busy_loop;
		}
		else
		{
			chan->fn_write = muggle_channel_mutex_write_mutex_read;
			chan->fn_wake = muggle_channel_wake_mutex;
			chan->fn_read = muggle_channel_read_mutex;
		}
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

	muggle_condition_variable_destroy(&chan->read_cv);
	muggle_mutex_destroy(&chan->read_mutex);
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

#endif
