/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "ringbuffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#if MUGGLE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <sched.h>
#endif
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/sync/futex.h"

enum
{
	MUGGLE_RINGBUFFER_WRITE_MODE_LOCK = 0,
	MUGGLE_RINGBUFFER_WRITE_MODE_SINGLE = 1,
	MUGGLE_RINGBUFFER_WRITE_MODE_BUSY_LOOP = 2,
	MUGGLE_RINGBUFFER_WRITE_MODE_MAX,
};

enum
{
	MUGGLE_RINGBUFFER_READ_MODE_WAIT = 0,
	MUGGLE_RINGBUFFER_READ_MODE_SINGLE_WAIT = 1,
	MUGGLE_RINGBUFFER_READ_MODE_BUSY_LOOP = 2,
	MUGGLE_RINGBUFFER_READ_MODE_LOCK = 3,
	MUGGLE_RINGBUFFER_READ_MODE_MAX,
};

typedef void (*fn_muggle_ringbuffer_write)(muggle_ringbuffer_t *r, void *data);
typedef void (*fn_muggle_ringbuffer_wake)(muggle_ringbuffer_t *r);
typedef void* (*fn_muggle_ringbuffer_read)(muggle_ringbuffer_t *r, muggle_atomic_int idx);

// convert flag to mode
static int muggle_ringbuffer_get_mode(int flag, int *w_mode, int *r_mode)
{
	// write mode
	if (flag & MUGGLE_RINGBUFFER_FLAG_SINGLE_WRITER)
	{
		*w_mode = MUGGLE_RINGBUFFER_WRITE_MODE_SINGLE;
	}
	else if (flag & MUGGLE_RINGBUFFER_FLAG_WRITE_BUSY_LOOP)
	{
		*w_mode = MUGGLE_RINGBUFFER_WRITE_MODE_BUSY_LOOP;
	}
	else
	{
		*w_mode = MUGGLE_RINGBUFFER_WRITE_MODE_LOCK;
	}

	// read mode
	if (flag & MUGGLE_RINGBUFFER_FLAG_SINGLE_READER)
	{
		if (flag & MUGGLE_RINGBUFFER_FLAG_READ_BUSY_LOOP)
		{
			*r_mode = MUGGLE_RINGBUFFER_READ_MODE_BUSY_LOOP;
		}
		else
		{
			*r_mode = MUGGLE_RINGBUFFER_READ_MODE_SINGLE_WAIT;
		}
	}
	else if (flag & MUGGLE_RINGBUFFER_FLAG_MSG_READ_ONCE)
	{
		if (flag & MUGGLE_RINGBUFFER_FLAG_READ_BUSY_LOOP)
		{
			return MUGGLE_ERR_INVALID_PARAM;
		}
		*r_mode = MUGGLE_RINGBUFFER_READ_MODE_LOCK;
	}
	else
	{
		if (flag & MUGGLE_RINGBUFFER_FLAG_READ_BUSY_LOOP)
		{
			*r_mode = MUGGLE_RINGBUFFER_READ_MODE_BUSY_LOOP;
		}
		else
		{
			*r_mode = MUGGLE_RINGBUFFER_READ_MODE_WAIT;
		}
	}

	return MUGGLE_OK;
}

// muggle ringbuffer write functions
inline static void muggle_ringbuffer_write_lock(muggle_ringbuffer_t *r, void *data)
{
	muggle_mutex_lock(&r->write_mutex);

	// assignment
	r->datas[IDX_IN_POW_OF_2_RING(r->cursor, r->capacity)] = data;

	// move cursor
	muggle_atomic_store(&r->cursor, r->cursor+1, muggle_memory_order_release);

	muggle_mutex_unlock(&r->write_mutex);
}

inline static void muggle_ringbuffer_write_single(muggle_ringbuffer_t *r, void *data)
{
	// assignment
	r->datas[IDX_IN_POW_OF_2_RING(r->cursor, r->capacity)] = data;

	// move cursor
	muggle_atomic_store(&r->cursor, r->cursor+1, muggle_memory_order_release);
}

inline static void muggle_ringbuffer_write_busy_loop(muggle_ringbuffer_t *r, void *data)
{
	// move next
	muggle_atomic_int idx = muggle_atomic_fetch_add(&r->next, 1, muggle_memory_order_relaxed);

	// assignment
	r->datas[IDX_IN_POW_OF_2_RING(idx, r->capacity)] = data;

	// move cursor
	while (muggle_atomic_load(&r->cursor, muggle_memory_order_relaxed) != idx)
	{
#if MUGGLE_PLATFORM_WINDOWS
		SwitchToThread();
#else
		sched_yield();
#endif
	}
	muggle_atomic_store(&r->cursor, idx + 1, muggle_memory_order_release);
}

// muggle ringbuffer wakeup functions
inline static void muggle_ringbuffer_wake_wait(muggle_ringbuffer_t *r)
{
	muggle_futex_wake_all(&r->cursor);
}

inline static void muggle_ringbuffer_wake_busy_loop(muggle_ringbuffer_t *r)
{
	// do nothing
}

inline static void muggle_ringbuffer_wake_single_wait(muggle_ringbuffer_t *r)
{
	muggle_futex_wake_one(&r->cursor);
}

inline static void muggle_ringbuffer_wake_lock(muggle_ringbuffer_t *r)
{
	muggle_futex_wake_one(&r->cursor);
}

// muggle ringbuffer read functions
inline static void* muggle_ringbuffer_read_wait(muggle_ringbuffer_t *r, muggle_atomic_int idx)
{
	muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(idx, r->capacity);
	muggle_atomic_int w_cursor;
	do {
		w_cursor = muggle_atomic_load(&r->cursor, muggle_memory_order_acquire);
		if (IDX_IN_POW_OF_2_RING(w_cursor, r->capacity) != r_pos)
		{
			return r->datas[r_pos];
		}

		muggle_futex_wait(&r->cursor, w_cursor, NULL);
	} while (1);

	return NULL;
}

inline static void* muggle_ringbuffer_read_busy_loop(muggle_ringbuffer_t *r, muggle_atomic_int idx)
{
	muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(idx, r->capacity);
	muggle_atomic_int w_cursor;

	do {
		w_cursor = muggle_atomic_load(&r->cursor, muggle_memory_order_acquire);
		if (IDX_IN_POW_OF_2_RING(w_cursor, r->capacity) != r_pos)
		{
			return r->datas[r_pos];
		}

#if MUGGLE_PLATFORM_WINDOWS
		SwitchToThread();
#else
		sched_yield();
#endif
	} while (1);

	return NULL;
}

inline static void* muggle_ringbuffer_read_lock(muggle_ringbuffer_t *r, muggle_atomic_int idx)
{
	void *ret = NULL;
	muggle_mutex_lock(&r->read_mutex);
	do {
		muggle_atomic_int w_cursor = muggle_atomic_load(&r->cursor, muggle_memory_order_acquire);
		muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(r->read_cursor, r->capacity);
		if (IDX_IN_POW_OF_2_RING(w_cursor, r->capacity) != r_pos)
		{
			ret = r->datas[r_pos];
			r->read_cursor++;
			break;
		}
		muggle_futex_wait(&r->cursor, w_cursor, NULL);
	} while (1);
	muggle_mutex_unlock(&r->read_mutex);

	return ret;
}


// write, wake and read callbacks
static fn_muggle_ringbuffer_write muggle_ringbuffer_write_functions[MUGGLE_RINGBUFFER_WRITE_MODE_MAX] = {
	muggle_ringbuffer_write_lock, // MUGGLE_RINGBUFFER_WRITE_MODE_LOCK 
	muggle_ringbuffer_write_single, // MUGGLE_RINGBUFFER_WRITE_MODE_SINGLE 
	muggle_ringbuffer_write_busy_loop // MUGGLE_RINGBUFFER_WRITE_MODE_BUSY_LOOP 
};

static fn_muggle_ringbuffer_wake muggle_ringbuffer_wake_functions[MUGGLE_RINGBUFFER_READ_MODE_MAX] = {
	muggle_ringbuffer_wake_wait, // MUGGLE_RINGBUFFER_READ_MODE_WAIT 
	muggle_ringbuffer_wake_single_wait, // MUGGLE_RINGBUFFER_READ_MODE_SINGLE_WAIT 
	muggle_ringbuffer_wake_busy_loop, // MUGGLE_RINGBUFFER_READ_MODE_BUSY_LOOP 
	muggle_ringbuffer_wake_lock, // MUGGLE_RINGBUFFER_READ_MODE_LOCK 
};

static fn_muggle_ringbuffer_read muggle_ringbuffer_read_functions[MUGGLE_RINGBUFFER_READ_MODE_MAX] = {
	muggle_ringbuffer_read_wait, // MUGGLE_RINGBUFFER_READ_MODE_WAIT 
	muggle_ringbuffer_read_wait, // MUGGLE_RINGBUFFER_READ_MODE_SINGLE_WAIT 
	muggle_ringbuffer_read_busy_loop, // MUGGLE_RINGBUFFER_READ_MODE_BUSY_LOOP 
	muggle_ringbuffer_read_lock, // MUGGLE_RINGBUFFER_READ_MODE_LOCK 
};

int muggle_ringbuffer_init(muggle_ringbuffer_t *r, muggle_atomic_int capacity, int flag)
{
	memset(r, 0, sizeof(muggle_ringbuffer_t));
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
	int ret = muggle_ringbuffer_get_mode(r->flag, &r->write_mode, &r->read_mode);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	r->next = 0;
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
		return ret;
	}

	ret = muggle_condition_variable_init(&r->read_cv);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	r->datas = (void**)malloc(sizeof(void*) * r->capacity);
	if (r->datas == NULL)
	{
		return MUGGLE_ERR_MEM_ALLOC;
	}

	return MUGGLE_OK;
}

int muggle_ringbuffer_destroy(muggle_ringbuffer_t *r)
{
	free(r->datas);
	muggle_mutex_destroy(&r->write_mutex);
	muggle_mutex_destroy(&r->read_mutex);
	muggle_condition_variable_destroy(&r->read_cv);
	return MUGGLE_OK;
}

int muggle_ringbuffer_write(muggle_ringbuffer_t *r, void *data)
{
	// write
	(*muggle_ringbuffer_write_functions[r->write_mode])(r, data);

	// wake
	(*muggle_ringbuffer_wake_functions[r->read_mode])(r);

	return MUGGLE_OK;
}

void* muggle_ringbuffer_read(muggle_ringbuffer_t *r, muggle_atomic_int idx)
{
	return (*muggle_ringbuffer_read_functions[r->read_mode])(r, idx);
}

