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
#include "muggle/c/sync/spinlock.h"

/***************** write lock *****************/

////////////////// MUGGLE_CHANNEL_FLAG_WRITE_FUTEX ////////////////// 

#if MUGGLE_C_HAVE_SYNC_OBJ

static void muggle_channel_write_sync_lock(muggle_channel_t *chan)
{
	muggle_synclock_lock(&chan->write_synclock);
}

static void muggle_channel_write_sync_unlock(muggle_channel_t *chan)
{
	muggle_synclock_unlock(&chan->write_synclock);
}

#endif

////////////////// MUGGLE_CHANNEL_FLAG_WRITE_MUTEX ////////////////// 

static void muggle_channel_write_mutex_lock(muggle_channel_t *chan)
{
	muggle_mutex_lock(chan->write_mutex);
}

static void muggle_channel_write_mutex_unlock(muggle_channel_t *chan)
{
	muggle_mutex_unlock(chan->write_mutex);
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
	MUGGLE_UNUSED(chan);
	// do nothing
}

static void muggle_channel_write_single_unlock(muggle_channel_t *chan)
{
	MUGGLE_UNUSED(chan);
	// do nothing
}

/***************** write & wake & read *****************/

////////////////// MUGGLE_CHANNEL_FLAG_READ_FUTEX ////////////////// 

#if MUGGLE_C_HAVE_SYNC_OBJ

static int muggle_channel_write_sync(muggle_channel_t *chan, void *data)
{
	// ensure not full
	//
	// NOTE:
	// Ensure everytime pos value in [0, cap - 1], don't increase cursor
	// and rely on integer wrap around. In *nix is ok, cause muggle_sync_t is
	// unsigned type in unix/linux, but in Windows, muggle_sync_t is signed 
	// integer, according to the c language standard, signed integer overflow
	// is undefined behavior
	muggle_sync_t rpos =
		muggle_atomic_load(&chan->read_cursor, muggle_memory_order_relaxed);
	muggle_sync_t wpos =
		IDX_IN_POW_OF_2_RING(chan->write_cursor + 1, chan->capacity);
	if (wpos == rpos)
	{
		return MUGGLE_ERR_FULL;
	}

	// write
	chan->blocks[chan->write_cursor].data = data;

	// NOTE: Blow first block's comment is Wrong! Wrong! Wrong!
	//
	// ------------------------------------------------------------------------
	// Wrong!!!  Wrong!!!  Wrong!!!
	// [Incorrect]no need to use atomic release here
	// * futex implementation
	//     memory barrier and read are part of futex(FUTEX_WAKE), so no need to
	//     use atomic release here.
	// * win synchapi implementation
	//     The following synchronization functions use the appropriate barriers 
	//     to ensure memory ordering
	//     * Functions that enter or leave critical sections
	//     * Functions that acquire or release SRW locks
	//     * One-time initialization begin and completion
	//     * EnterSynchronizationBarrier function
	//     * Functions that signal synchronization objects
	//     * Wait functions
	//     * Interlocked functions (except functions with NoFence suffix, or 
	//       intrinsics with _nf suffix)
	// chan->write_cursor = wpos;
	// ------------------------------------------------------------------------
	// 
	// Some case are left out, so must use atomic_store. 
	// Consider this situation below
	// ------------------------------------------------------------------------
	//     Write Thread    |    Read Thread
	//      set data       |
	//   move write cursor |
	//                     | compare read and write cursor, and read data
	// ------------------------------------------------------------------------
	//
	// In this case, if without atomic_store in move write cursor, reader may
	// get correct write cursor with incorrect data in the position

	muggle_atomic_store(
		&chan->write_cursor, wpos, muggle_memory_order_release);

	return MUGGLE_OK;
}

static void muggle_channel_wake_sync(muggle_channel_t *chan)
{
	muggle_sync_wake_one(&chan->write_cursor);
}

static void* muggle_channel_read_sync(muggle_channel_t *chan)
{
	muggle_sync_t rpos =
		IDX_IN_POW_OF_2_RING(chan->read_cursor + 1, chan->capacity);
	muggle_sync_t wpos;
	while (1)
	{
		wpos = muggle_atomic_load(
			&chan->write_cursor, muggle_memory_order_acquire);
		if (wpos != rpos)
		{
			void *data = chan->blocks[rpos].data;
			muggle_atomic_store(
				&chan->read_cursor, rpos, muggle_memory_order_relaxed);
			return data;
		}

		muggle_sync_wait(&chan->write_cursor, wpos, NULL);
	}

	return NULL;
}

#endif

////////////////// MUGGLE_CHANNEL_FLAG_READ_MUTEX ////////////////// 

static int muggle_channel_write_mutex(muggle_channel_t *chan, void *data)
{
	muggle_mutex_lock(chan->read_mutex);

	muggle_sync_t wpos =
		IDX_IN_POW_OF_2_RING(chan->write_cursor + 1, chan->capacity);

	if (wpos == chan->read_cursor)
	{
		muggle_mutex_unlock(chan->read_mutex);
		return MUGGLE_ERR_FULL;
	}

	chan->blocks[chan->write_cursor].data = data;
	chan->write_cursor = wpos;

	muggle_mutex_unlock(chan->read_mutex);

	return MUGGLE_OK;
}

static void muggle_channel_wake_mutex(muggle_channel_t *chan)
{
	muggle_condition_variable_notify_one(chan->read_cv);
}

static void* muggle_channel_read_mutex(muggle_channel_t *chan)
{
	muggle_mutex_lock(chan->read_mutex);

	while (1)
	{
		muggle_sync_t rpos =
			IDX_IN_POW_OF_2_RING(chan->read_cursor + 1, chan->capacity);
		if (rpos != chan->write_cursor)
		{
			void *data = chan->blocks[rpos].data;
			chan->read_cursor = rpos;
			muggle_mutex_unlock(chan->read_mutex);
			return data;
		}

		muggle_condition_variable_wait(
			chan->read_cv, chan->read_mutex, NULL);
	}
}

////////////////// MUGGLE_CHANNEL_FLAG_READ_BUSY ////////////////// 

static int muggle_channel_write_busy(muggle_channel_t *chan, void *data)
{
	muggle_sync_t rpos =
		muggle_atomic_load(&chan->read_cursor, muggle_memory_order_relaxed);
	muggle_sync_t wpos =
		IDX_IN_POW_OF_2_RING(chan->write_cursor + 1, chan->capacity);
	if (wpos == rpos)
	{
		return MUGGLE_ERR_FULL;
	}

	chan->blocks[chan->write_cursor].data = data;
	muggle_atomic_store(
		&chan->write_cursor, wpos, muggle_memory_order_release);

	return MUGGLE_OK;
}

static void muggle_channel_wake_busy(muggle_channel_t *chan)
{
	MUGGLE_UNUSED(chan);
	// do nothing
}

static void* muggle_channel_read_busy(muggle_channel_t *chan)
{
	muggle_sync_t rpos =
		IDX_IN_POW_OF_2_RING(chan->read_cursor + 1, chan->capacity);
	muggle_sync_t wpos;
	while (1)
	{
		wpos = muggle_atomic_load(
			&chan->write_cursor, muggle_memory_order_acquire);
		if (wpos != rpos)
		{
			void *data = chan->blocks[rpos].data;
			muggle_atomic_store(
				&chan->read_cursor, rpos, muggle_memory_order_relaxed);
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

int muggle_channel_init(
	muggle_channel_t *chan, muggle_sync_t capacity, int flags)
{
	memset(chan, 0, sizeof(*chan));

	int ret = 0;

	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	capacity = (muggle_sync_t)next_pow_of_2((uint64_t)capacity);
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	chan->capacity = capacity;

	// get flags
	int w_flags = flags & MUGGLE_CHANNEL_FLAG_MASK_W;
	int r_flags = flags & MUGGLE_CHANNEL_FLAG_MASK_R;
#if !MUGGLE_C_HAVE_SYNC_OBJ
	if (w_flags == MUGGLE_CHANNEL_FLAG_WRITE_SYNC)
	{
		w_flags = MUGGLE_CHANNEL_FLAG_WRITE_MUTEX;
	}
	if (r_flags == MUGGLE_CHANNEL_FLAG_READ_SYNC)
	{
		r_flags = MUGGLE_CHANNEL_FLAG_READ_MUTEX;
	}
	flags = w_flags | r_flags;
#endif

	chan->flags = flags;

	// set write lock
	switch (w_flags)
	{
#if MUGGLE_C_HAVE_SYNC_OBJ
	case MUGGLE_CHANNEL_FLAG_WRITE_SYNC:
		{
			muggle_synclock_init(&chan->write_synclock);
			chan->fn_lock = muggle_channel_write_sync_lock;
			chan->fn_unlock = muggle_channel_write_sync_unlock;
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

			chan->write_mutex =
				(muggle_mutex_t*)malloc(sizeof(muggle_mutex_t));
			if (chan->write_mutex == NULL)
			{
				goto channel_init_except;
			}

			ret = muggle_mutex_init(chan->write_mutex);
			if (ret != MUGGLE_OK)
			{
				free(chan->write_mutex);
				chan->write_mutex = NULL;

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
#if MUGGLE_C_HAVE_SYNC_OBJ
	case MUGGLE_CHANNEL_FLAG_READ_SYNC:
		{
			chan->fn_write = muggle_channel_write_sync;
			chan->fn_wake = muggle_channel_wake_sync;
			chan->fn_read = muggle_channel_read_sync;
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

			chan->read_mutex = (muggle_mutex_t*)malloc(sizeof(muggle_mutex_t));
			if (chan->read_mutex == NULL)
			{
				goto channel_init_except;
			}

			ret = muggle_mutex_init(chan->read_mutex);
			if (ret != MUGGLE_OK)
			{
				free(chan->read_mutex);
				chan->read_mutex = NULL;

				goto channel_init_except;
			}
			chan->init_flags |= CHANNEL_INIT_READ_MUTEX;

			chan->read_cv =(muggle_condition_variable_t*)malloc(
				sizeof(muggle_condition_variable_t));
			if (chan->read_cv == NULL)
			{
				goto channel_init_except;
			}

			ret = muggle_condition_variable_init(chan->read_cv);
			if (ret != MUGGLE_OK)
			{
				free(chan->read_cv);
				chan->read_cv = NULL;

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

	chan->blocks =(muggle_channel_block_t*)malloc(
		sizeof(muggle_channel_block_t) * capacity);
	if (chan->blocks == NULL)
	{
		ret = MUGGLE_ERR_MEM_ALLOC;
		goto channel_init_except;
	}

	for (muggle_sync_t i = 0; i < capacity; i++)
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

	if (chan->read_cv)
	{
		muggle_condition_variable_destroy(chan->read_cv);
		free(chan->read_cv);
		chan->read_cv = NULL;
	}
	if (chan->read_mutex)
	{
		muggle_mutex_destroy(chan->read_mutex);
		free(chan->read_mutex);
		chan->read_mutex = NULL;
	}
	if (chan->write_mutex)
	{
		muggle_mutex_destroy(chan->write_mutex);
		free(chan->write_mutex);
		chan->write_mutex = NULL;
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
