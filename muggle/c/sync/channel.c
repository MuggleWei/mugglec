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
#include "muggle/c/sync/spinlock.h"

/***************** write lock *****************/

////////////////// MUGGLE_CHANNEL_FLAG_WRITE_FUTEX ////////////////// 

#if MUGGLE_SUPPORT_FUTEX

static void muggle_channel_write_futex_lock(muggle_channel_t *chan)
{
	muggle_futex_lock(&chan->write_futex);
}

static void muggle_channel_write_futex_unlock(muggle_channel_t *chan)
{
	muggle_futex_unlock(&chan->write_futex);
	muggle_futex_wake_one(&chan->write_futex);
}

#endif

////////////////// MUGGLE_CHANNEL_FLAG_WRITE_MUTEX ////////////////// 

static void muggle_channel_write_mutex_lock(muggle_channel_t *chan)
{
	muggle_mutex_lock(&chan->write_mutex);
}

static void muggle_channel_write_mutex_unlock(muggle_channel_t *chan)
{
	muggle_mutex_unlock(&chan->write_mutex);
}

////////////////// MUGGLE_CHANNEL_FLAG_WRITE_SPIN ////////////////// 

static void muggle_channel_write_spinlock_lock(muggle_channel_t *chan)
{
	muggle_spinlock_lock(&chan->write_spinlock);
}

static void muggle_channel_write_spinlock_unlock(muggle_channel_t *chan)
{
	muggle_spinlock_unlock(&chan->write_spinlock);
}

////////////////// MUGGLE_CHANNEL_FLAG_WRITE_SINGLE ////////////////// 

static void muggle_channel_write_single_lock(muggle_channel_t *chan)
{
	// do nothing
}

static void muggle_channel_write_single_unlock(muggle_channel_t *chan)
{
	// do nothing
}

/***************** write & wake & read *****************/

////////////////// MUGGLE_CHANNEL_FLAG_READ_FUTEX ////////////////// 

#if MUGGLE_SUPPORT_FUTEX

static int muggle_channel_write_futex(muggle_channel_t *chan, void *data)
{
	if (chan->write_cursor + 1 == muggle_atomic_load(&chan->read_cursor, muggle_memory_order_relaxed))
	{
		return MUGGLE_ERR_FULL;
	}

	chan->blocks[IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity)].data = data;
	muggle_atomic_store(&chan->write_cursor, chan->write_cursor + 1, muggle_memory_order_release);

	return MUGGLE_OK;
}

static void muggle_channel_wake_futex(muggle_channel_t *chan)
{
	muggle_futex_wake_one(&chan->write_cursor);
}

static void* muggle_channel_read_futex(muggle_channel_t *chan)
{
	muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(chan->read_cursor + 1, chan->capacity);
	muggle_atomic_int w_cursor;
	while (1)
	{
		w_cursor = muggle_atomic_load(&chan->write_cursor, muggle_memory_order_acquire);
		if (IDX_IN_POW_OF_2_RING(w_cursor, chan->capacity) != r_pos)
		{
			void *data = chan->blocks[r_pos].data;
			muggle_atomic_fetch_add(&chan->read_cursor, 1, muggle_memory_order_relaxed);
			return data;
		}

		muggle_futex_wait(&chan->write_cursor, w_cursor, NULL);
	}

	return NULL;
}

#endif

////////////////// MUGGLE_CHANNEL_FLAG_READ_MUTEX ////////////////// 

static int muggle_channel_write_mutex(muggle_channel_t *chan, void *data)
{
	muggle_mutex_lock(&chan->read_mutex);

	if (chan->write_cursor + 1 == chan->read_cursor)
	{
		muggle_mutex_unlock(&chan->read_mutex);
		return MUGGLE_ERR_FULL;
	}

	chan->blocks[IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity)].data = data;
	chan->write_cursor++;

	muggle_mutex_unlock(&chan->read_mutex);

	return MUGGLE_OK;
}

static void muggle_channel_wake_mutex(muggle_channel_t *chan)
{
	muggle_condition_variable_notify_one(&chan->read_cv);
}

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

////////////////// MUGGLE_CHANNEL_FLAG_READ_BUSY ////////////////// 

static int muggle_channel_write_busy(muggle_channel_t *chan, void *data)
{
	if (chan->write_cursor + 1 == muggle_atomic_load(&chan->read_cursor, muggle_memory_order_relaxed))
	{
		return MUGGLE_ERR_FULL;
	}

	chan->blocks[IDX_IN_POW_OF_2_RING(chan->write_cursor, chan->capacity)].data = data;
	muggle_atomic_store(&chan->write_cursor, chan->write_cursor + 1, muggle_memory_order_release);

	return MUGGLE_OK;
}

static void muggle_channel_wake_busy(muggle_channel_t *chan)
{
	// do nothing
}

static void* muggle_channel_read_busy(muggle_channel_t *chan)
{
	muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(chan->read_cursor + 1, chan->capacity);
	while (1)
	{
		if (IDX_IN_POW_OF_2_RING(muggle_atomic_load(&chan->write_cursor, muggle_memory_order_acquire), chan->capacity) != r_pos)
		{
			void *data = chan->blocks[r_pos].data;
			muggle_atomic_fetch_add(&chan->read_cursor, 1, muggle_memory_order_relaxed);
			return data;
		}

		muggle_thread_yield();
	}

	return NULL;
}

/***************** channel functions *****************/

enum
{
	CHANNEL_INIT_WRITE_MUTEX = 1 << 0,
	CHANNEL_INIT_READ_MUTEX  = 1 << 1,
	CHANNEL_INIT_READ_CV     = 1 << 2,
};

int muggle_channel_init(muggle_channel_t *chan, muggle_atomic_int capacity, int flags)
{
	memset(chan, 0, sizeof(*chan));

	int ret = 0;

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

	// get flags
	int w_flags = flags & MUGGLE_CHANNEL_FLAG_MASK_W;
	int r_flags = flags & MUGGLE_CHANNEL_FLAG_MASK_R;
#if ! defined(MUGGLE_SUPPORT_FUTEX)
	if (w_flags == MUGGLE_CHANNEL_FLAG_WRITE_FUTEX)
	{
		w_flags = MUGGLE_CHANNEL_FLAG_WRITE_MUTEX;
	}
	if (r_flags == MUGGLE_CHANNEL_FLAG_READ_FUTEX)
	{
		r_flags = MUGGLE_CHANNEL_FLAG_READ_MUTEX;
	}
	flags = w_flags | r_flags;
#endif

	chan->flags = flags;

	// set write lock
	switch (w_flags)
	{
#if MUGGLE_SUPPORT_FUTEX
	case MUGGLE_CHANNEL_FLAG_WRITE_FUTEX:
		{
			muggle_futex_init(&chan->write_futex);
			chan->fn_lock = muggle_channel_write_futex_lock;
			chan->fn_unlock = muggle_channel_write_futex_unlock;
		}break;
#endif
	case MUGGLE_CHANNEL_FLAG_WRITE_SPIN:
		{
			muggle_spinlock_init(&chan->write_spinlock);
			chan->fn_lock = muggle_channel_write_spinlock_lock;
			chan->fn_unlock = muggle_channel_write_spinlock_unlock;
		}break;
	case MUGGLE_CHANNEL_FLAG_WRITE_SINGLE:
		{
			chan->fn_lock = muggle_channel_write_single_lock;
			chan->fn_unlock = muggle_channel_write_single_unlock;
		}break;
	case MUGGLE_CHANNEL_FLAG_WRITE_MUTEX:
	default:
		{
			// if user set invalid write flag, use mutex
			w_flags = MUGGLE_CHANNEL_FLAG_WRITE_MUTEX;
			chan->flags = w_flags | r_flags;

			ret = muggle_mutex_init(&chan->write_mutex);
			if (ret != MUGGLE_OK)
			{
				goto channel_init_except;
			}
			chan->init_flags |= CHANNEL_INIT_WRITE_MUTEX;

			chan->fn_lock = muggle_channel_write_mutex_lock;
			chan->fn_unlock = muggle_channel_write_mutex_unlock;
		}break;
	}

	// set reader
	switch (r_flags)
	{
#if MUGGLE_SUPPORT_FUTEX
	case MUGGLE_CHANNEL_FLAG_READ_FUTEX:
		{
			chan->fn_write = muggle_channel_write_futex;
			chan->fn_wake = muggle_channel_wake_futex;
			chan->fn_read = muggle_channel_read_futex;
		}break;
#endif
	case MUGGLE_CHANNEL_FLAG_READ_BUSY:
		{
			chan->fn_write = muggle_channel_write_busy;
			chan->fn_wake = muggle_channel_wake_busy;
			chan->fn_read = muggle_channel_read_busy;
		}break;
	case MUGGLE_CHANNEL_FLAG_READ_MUTEX:
	default:
		{
			// if user set invalid read flag, use mutex
			r_flags = MUGGLE_CHANNEL_FLAG_READ_MUTEX;
			chan->flags = w_flags | r_flags;

			ret = muggle_mutex_init(&chan->read_mutex);
			if (ret != MUGGLE_OK)
			{
				goto channel_init_except;
			}
			chan->init_flags |= CHANNEL_INIT_READ_MUTEX;

			ret = muggle_condition_variable_init(&chan->read_cv);
			if (ret != MUGGLE_OK)
			{
				goto channel_init_except;
			}
			chan->init_flags |= CHANNEL_INIT_READ_CV;

			chan->fn_write = muggle_channel_write_mutex;
			chan->fn_wake = muggle_channel_wake_mutex;
			chan->fn_read = muggle_channel_read_mutex;
		}break;
	}

	chan->write_cursor = 0;
	chan->read_cursor = capacity - 1;

	chan->blocks = (muggle_channel_block_t*)malloc(sizeof(muggle_channel_block_t) * capacity);
	if (chan->blocks == NULL)
	{
		ret = MUGGLE_ERR_MEM_ALLOC;
		goto channel_init_except;
	}

	for (muggle_atomic_int i = 0; i < capacity; i++)
	{
		memset(&chan->blocks[i], 0, sizeof(muggle_channel_block_t));
	}

	return MUGGLE_OK;

channel_init_except:
	muggle_channel_destroy(chan);

	return ret;
}

void muggle_channel_destroy(muggle_channel_t *chan)
{
	if (chan->blocks)
	{
		free(chan->blocks);
		chan->blocks = NULL;
	}

	if (chan->init_flags & CHANNEL_INIT_READ_CV)
	{
		muggle_condition_variable_destroy(&chan->read_cv);
	}
	if (chan->init_flags & CHANNEL_INIT_READ_MUTEX)
	{
		muggle_mutex_destroy(&chan->read_mutex);
	}
	if (chan->init_flags & CHANNEL_INIT_WRITE_MUTEX)
	{
		muggle_mutex_destroy(&chan->write_mutex);
	}
}

int muggle_channel_write(muggle_channel_t *chan, void *data)
{
	int ret = 0;

	chan->fn_lock(chan);
	ret = chan->fn_write(chan, data);
	chan->fn_unlock(chan);

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
