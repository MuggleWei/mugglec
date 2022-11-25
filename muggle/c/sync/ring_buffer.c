/******************************************************************************
 *  @file         ring_buffer.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec ring buffer
 *****************************************************************************/

#include "ring_buffer.h"
#include "muggle/c/base/err.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/sync/futex.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if MUGGLE_C_HAVE_SYNC_OBJ
	#define MUGGLE_RING_BUFFER_USE_SYNC 1
#endif /* MUGGLE_C_HAVE_SYNC_OBJ */

enum
{
	MUGGLE_RING_BUFFER_WRITE_MODE_LOCK = 0,
	MUGGLE_RING_BUFFER_WRITE_MODE_SINGLE = 1,
	MUGGLE_RING_BUFFER_WRITE_MODE_MAX,
};

enum
{
	MUGGLE_RING_BUFFER_READ_MODE_WAIT = 0,
	MUGGLE_RING_BUFFER_READ_MODE_SINGLE_WAIT = 1,
	MUGGLE_RING_BUFFER_READ_MODE_BUSY_LOOP = 2,
	MUGGLE_RING_BUFFER_READ_MODE_ONCE = 3,
	MUGGLE_RING_BUFFER_READ_MODE_MAX,
};

typedef void (*fn_muggle_ring_buffer_write)(
	muggle_ring_buffer_t *r, void *data);
typedef void (*fn_muggle_ring_buffer_wake)(muggle_ring_buffer_t *r);
typedef void *(*fn_muggle_ring_buffer_read)(
	muggle_ring_buffer_t *r, muggle_sync_t idx);

// convert flag to mode
static int muggle_ring_buffer_get_mode(int flag, int *w_mode, int *r_mode)
{
	// write mode
	if (flag & MUGGLE_RING_BUFFER_FLAG_SINGLE_WRITER)
	{
		*w_mode = MUGGLE_RING_BUFFER_WRITE_MODE_SINGLE;
	}
	else
	{
		*w_mode = MUGGLE_RING_BUFFER_WRITE_MODE_LOCK;
	}

	// read mode
	if (flag & MUGGLE_RING_BUFFER_FLAG_SINGLE_READER)
	{
		if (flag & MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP)
		{
			*r_mode = MUGGLE_RING_BUFFER_READ_MODE_BUSY_LOOP;
		}
		else
		{
			*r_mode = MUGGLE_RING_BUFFER_READ_MODE_SINGLE_WAIT;
		}
	}
	else if (flag & MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE)
	{
		if (flag & MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP)
		{
			return MUGGLE_ERR_INVALID_PARAM;
		}
		*r_mode = MUGGLE_RING_BUFFER_READ_MODE_ONCE;
	}
	else
	{
		if (flag & MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP)
		{
			*r_mode = MUGGLE_RING_BUFFER_READ_MODE_BUSY_LOOP;
		}
		else
		{
			*r_mode = MUGGLE_RING_BUFFER_READ_MODE_WAIT;
		}
	}

	return MUGGLE_OK;
}

/***************** lock & write *****************/

// muggle ring_buffer write functions
inline static void muggle_ring_buffer_write_lock(muggle_ring_buffer_t *r, void *data)
{
	muggle_mutex_lock(&r->write_mutex);

	// assignment
	r->blocks[r->cursor].data = data;

	// move cursor
	muggle_sync_t rpos = IDX_IN_POW_OF_2_RING(r->cursor + 1, r->capacity);
	muggle_atomic_store(&r->cursor, rpos, muggle_memory_order_release);

	muggle_mutex_unlock(&r->write_mutex);
}

inline static void muggle_ring_buffer_write_single(muggle_ring_buffer_t *r, void *data)
{
	// assignment
	r->blocks[r->cursor].data = data;

	// move cursor
	muggle_sync_t rpos = IDX_IN_POW_OF_2_RING(r->cursor + 1, r->capacity);
	muggle_atomic_store(&r->cursor, rpos, muggle_memory_order_release);
}

/***************** wake *****************/

// muggle ring_buffer wakeup functions
inline static void muggle_ring_buffer_wake_wait(muggle_ring_buffer_t *r)
{
#if MUGGLE_RING_BUFFER_USE_SYNC
	muggle_sync_wake_all(&r->cursor);
#else
	muggle_condition_variable_notify_all(&r->read_cv);
#endif
}

inline static void muggle_ring_buffer_wake_single_wait(muggle_ring_buffer_t *r)
{
#if MUGGLE_RING_BUFFER_USE_SYNC
	muggle_sync_wake_one(&r->cursor);
#else
	muggle_condition_variable_notify_one(&r->read_cv);
#endif
}

inline static void muggle_ring_buffer_wake_busy_loop(muggle_ring_buffer_t *r)
{
	MUGGLE_UNUSED(r);
	// do nothing
}

inline static void muggle_ring_buffer_wake_lock(muggle_ring_buffer_t *r)
{
#if MUGGLE_RING_BUFFER_USE_SYNC
	muggle_sync_wake_one(&r->cursor);
#else
	muggle_condition_variable_notify_one(&r->read_cv);
#endif
}

/***************** read *****************/
// muggle ring_buffer read functions
inline static void* muggle_ring_buffer_read_wait(
	muggle_ring_buffer_t *r, muggle_sync_t idx)
{
#if MUGGLE_RING_BUFFER_USE_SYNC
	muggle_atomic_int rpos = IDX_IN_POW_OF_2_RING(idx, r->capacity);
	muggle_atomic_int wpos;
	do
	{
		wpos = muggle_atomic_load(&r->cursor, muggle_memory_order_acquire);
		if (wpos != rpos)
		{
			return r->blocks[rpos].data;
		}

		muggle_sync_wait(&r->cursor, wpos, NULL);
	} while (1);
#else
	muggle_atomic_int rpos = IDX_IN_POW_OF_2_RING(idx, r->capacity);
	muggle_atomic_int wpos;

	muggle_mutex_lock(&r->read_mutex);
	do {
		wpos = muggle_atomic_load(&r->cursor, muggle_memory_order_acquire);
		if (wpos != rpos)
		{
			void *data = r->blocks[rpos].data;
			muggle_mutex_unlock(&r->read_mutex);
			return data;
		}

		muggle_condition_variable_wait(&r->read_cv, &r->read_mutex, NULL);
	} while (1);
#endif

	return NULL;
}

inline static void *muggle_ring_buffer_read_busy_loop(
	muggle_ring_buffer_t *r, muggle_sync_t idx)
{
	muggle_atomic_int rpos = IDX_IN_POW_OF_2_RING(idx, r->capacity);
	muggle_atomic_int wpos;

	do
	{
		wpos = muggle_atomic_load(&r->cursor, muggle_memory_order_acquire);
		if (wpos != rpos)
		{
			return r->blocks[rpos].data;
		}

		muggle_thread_yield();
	} while (1);

	return NULL;
}

inline static void *muggle_ring_buffer_read_once(
	muggle_ring_buffer_t *r, muggle_sync_t idx)
{
	MUGGLE_UNUSED(idx);

	void *ret = NULL;
	muggle_mutex_lock(&r->read_mutex);
	do
	{
		muggle_sync_t wpos = muggle_atomic_load(
			&r->cursor, muggle_memory_order_acquire);
		if (r->read_cursor != wpos)
		{
			ret = r->blocks[r->read_cursor].data;
			r->read_cursor =
				IDX_IN_POW_OF_2_RING(r->read_cursor+1, r->capacity);
			break;
		}

#if MUGGLE_RING_BUFFER_USE_SYNC
		muggle_sync_wait(&r->cursor, wpos, NULL);
#else
		muggle_condition_variable_wait(&r->read_cv, &r->read_mutex, NULL);
#endif
	} while (1);
	muggle_mutex_unlock(&r->read_mutex);

	return ret;
}

// write, wake and read callbacks
static fn_muggle_ring_buffer_write muggle_ring_buffer_write_functions[MUGGLE_RING_BUFFER_WRITE_MODE_MAX] = {
	muggle_ring_buffer_write_lock, // MUGGLE_RING_BUFFER_WRITE_MODE_LOCK
	muggle_ring_buffer_write_single, // MUGGLE_RING_BUFFER_WRITE_MODE_SINGLE
};

static fn_muggle_ring_buffer_wake muggle_ring_buffer_wake_functions[MUGGLE_RING_BUFFER_READ_MODE_MAX] = {
	muggle_ring_buffer_wake_wait, // MUGGLE_RING_BUFFER_READ_MODE_WAIT
	muggle_ring_buffer_wake_single_wait, // MUGGLE_RING_BUFFER_READ_MODE_SINGLE_WAIT
	muggle_ring_buffer_wake_busy_loop, // MUGGLE_RING_BUFFER_READ_MODE_BUSY_LOOP
	muggle_ring_buffer_wake_lock, // MUGGLE_RING_BUFFER_READ_MODE_ONCE
};

static fn_muggle_ring_buffer_read muggle_ring_buffer_read_functions[MUGGLE_RING_BUFFER_READ_MODE_MAX] = {
	muggle_ring_buffer_read_wait, // MUGGLE_RING_BUFFER_READ_MODE_WAIT
	muggle_ring_buffer_read_wait, // MUGGLE_RING_BUFFER_READ_MODE_SINGLE_WAIT
	muggle_ring_buffer_read_busy_loop, // MUGGLE_RING_BUFFER_READ_MODE_BUSY_LOOP
	muggle_ring_buffer_read_once, // MUGGLE_RING_BUFFER_READ_MODE_ONCE
};

int muggle_ring_buffer_init(
	muggle_ring_buffer_t *r, muggle_sync_t capacity, int flag)
{
	memset(r, 0, sizeof(muggle_ring_buffer_t));
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	r->capacity = (muggle_atomic_int)next_pow_of_2((uint64_t)capacity);
	if (r->capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	r->flag = flag;
	int ret = muggle_ring_buffer_get_mode(r->flag, &r->write_mode, &r->read_mode);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	r->cursor = 0;
	r->read_cursor = 0;

	ret = muggle_mutex_init(&r->write_mutex);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	ret = muggle_mutex_init(&r->read_mutex);
	if (ret != MUGGLE_OK)
	{
		muggle_mutex_destroy(&r->write_mutex);
		return ret;
	}

	ret = muggle_condition_variable_init(&r->read_cv);
	if (ret != MUGGLE_OK)
	{
		muggle_mutex_destroy(&r->write_mutex);
		muggle_mutex_destroy(&r->read_mutex);
		return ret;
	}

	r->blocks = (muggle_ring_buffer_block_t*)malloc(sizeof(muggle_ring_buffer_block_t) * r->capacity);
	if (r->blocks == NULL)
	{
		muggle_mutex_destroy(&r->write_mutex);
		muggle_mutex_destroy(&r->read_mutex);
		muggle_condition_variable_destroy(&r->read_cv);
		return MUGGLE_ERR_MEM_ALLOC;
	}

	return MUGGLE_OK;
}

int muggle_ring_buffer_destroy(muggle_ring_buffer_t *r)
{
	free(r->blocks);
	muggle_mutex_destroy(&r->write_mutex);
	muggle_mutex_destroy(&r->read_mutex);
	muggle_condition_variable_destroy(&r->read_cv);
	return MUGGLE_OK;
}

int muggle_ring_buffer_write(muggle_ring_buffer_t *r, void *data)
{
	// write
	(*muggle_ring_buffer_write_functions[r->write_mode])(r, data);

	// wake
	(*muggle_ring_buffer_wake_functions[r->read_mode])(r);

	return MUGGLE_OK;
}

void *muggle_ring_buffer_read(muggle_ring_buffer_t *r, uint32_t idx)
{
	muggle_sync_t pos = IDX_IN_POW_OF_2_RING(idx, r->capacity);
	return (*muggle_ring_buffer_read_functions[r->read_mode])(r, pos);
}
