#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/base/thread.h"

#ifndef MUGGLE_SUPPORT_FUTEX

enum
{
	MUGGLE_RING_BUFFER_WRITE_MODE_LOCK = 0,
	MUGGLE_RING_BUFFER_WRITE_MODE_SINGLE = 1,
	MUGGLE_RING_BUFFER_WRITE_MODE_BUSY_LOOP = 2,
	MUGGLE_RING_BUFFER_WRITE_MODE_MAX,
};

enum
{
	MUGGLE_RING_BUFFER_READ_MODE_WAIT = 0,
	MUGGLE_RING_BUFFER_READ_MODE_SINGLE_WAIT = 1,
	MUGGLE_RING_BUFFER_READ_MODE_BUSY_LOOP = 2,
	MUGGLE_RING_BUFFER_READ_MODE_LOCK = 3,
	MUGGLE_RING_BUFFER_READ_MODE_MAX,
};

typedef void (*fn_muggle_ring_buffer_write)(muggle_ring_buffer_t *r, void *data);
typedef void (*fn_muggle_ring_buffer_wake)(muggle_ring_buffer_t *r);
typedef void* (*fn_muggle_ring_buffer_read)(muggle_ring_buffer_t *r, muggle_atomic_int idx);

// convert flag to mode
static int muggle_ring_buffer_get_mode(int flag, int *w_mode, int *r_mode)
{
	// write mode
	if (flag & MUGGLE_RING_BUFFER_FLAG_SINGLE_WRITER)
	{
		*w_mode = MUGGLE_RING_BUFFER_WRITE_MODE_SINGLE;
	}
	else if (flag & MUGGLE_RING_BUFFER_FLAG_WRITE_BUSY_LOOP)
	{
		*w_mode = MUGGLE_RING_BUFFER_WRITE_MODE_BUSY_LOOP;
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
		*r_mode = MUGGLE_RING_BUFFER_READ_MODE_LOCK;
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

// muggle ring_buffer write functions
inline static void muggle_ring_buffer_write_lock(muggle_ring_buffer_t *r, void *data)
{
	muggle_mutex_lock(&r->mtx);

	// assignment
	r->datas[IDX_IN_POW_OF_2_RING(r->cursor, r->capacity)] = data;

	// move cursor
	muggle_atomic_store(&r->cursor, r->cursor+1, muggle_memory_order_release);

	muggle_mutex_unlock(&r->mtx);
}

inline static void muggle_ring_buffer_write_single(muggle_ring_buffer_t *r, void *data)
{
	// assignment
	r->datas[IDX_IN_POW_OF_2_RING(r->cursor, r->capacity)] = data;

	// move cursor
	muggle_atomic_store(&r->cursor, r->cursor+1, muggle_memory_order_release);
}

inline static void muggle_ring_buffer_write_busy_loop(muggle_ring_buffer_t *r, void *data)
{
	// move next
	muggle_atomic_int idx = muggle_atomic_fetch_add(&r->next, 1, muggle_memory_order_relaxed);

	// assignment
	r->datas[IDX_IN_POW_OF_2_RING(idx, r->capacity)] = data;

	// move cursor
	muggle_atomic_int cur_idx = idx;
	while (!muggle_atomic_cmp_exch_weak(&r->cursor, &cur_idx, idx + 1, muggle_memory_order_release)
			&& cur_idx != idx)
	{
		muggle_thread_yield();
		cur_idx = idx;
	}
}

// muggle ring_buffer wakeup functions
inline static void muggle_ring_buffer_wake_wait(muggle_ring_buffer_t *r)
{
	muggle_condition_variable_notify_all(&r->cv);
}

inline static void muggle_ring_buffer_wake_busy_loop(muggle_ring_buffer_t *r)
{
	// do nothing
}

inline static void muggle_ring_buffer_wake_single_wait(muggle_ring_buffer_t *r)
{
	muggle_condition_variable_notify_one(&r->cv);
}

inline static void muggle_ring_buffer_wake_lock(muggle_ring_buffer_t *r)
{
	muggle_condition_variable_notify_one(&r->cv);
}

// muggle ring_buffer read functions
inline static void* muggle_ring_buffer_read_wait(muggle_ring_buffer_t *r, muggle_atomic_int idx)
{
	muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(idx, r->capacity);
	muggle_atomic_int w_cursor;

	muggle_mutex_lock(&r->mtx);
	do {
		w_cursor = muggle_atomic_load(&r->cursor, muggle_memory_order_acquire);
		if (IDX_IN_POW_OF_2_RING(w_cursor, r->capacity) != r_pos)
		{
			void *data = r->datas[r_pos];
			muggle_mutex_unlock(&r->mtx);
			return data;
		}

		muggle_condition_variable_wait(&r->cv, &r->mtx, NULL);
	} while (1);

	return NULL;
}

inline static void* muggle_ring_buffer_read_busy_loop(muggle_ring_buffer_t *r, muggle_atomic_int idx)
{
	muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(idx, r->capacity);
	muggle_atomic_int w_cursor;

	do {
		w_cursor = muggle_atomic_load(&r->cursor, muggle_memory_order_acquire);
		if (IDX_IN_POW_OF_2_RING(w_cursor, r->capacity) != r_pos)
		{
			return r->datas[r_pos];
		}

		muggle_thread_yield();
	} while (1);

	return NULL;
}

inline static void* muggle_ring_buffer_read_lock(muggle_ring_buffer_t *r, muggle_atomic_int idx)
{
	void *ret = NULL;
	muggle_mutex_lock(&r->mtx);
	do {
		muggle_atomic_int w_cursor = muggle_atomic_load(&r->cursor, muggle_memory_order_acquire);
		muggle_atomic_int r_pos = IDX_IN_POW_OF_2_RING(r->read_cursor, r->capacity);
		if (IDX_IN_POW_OF_2_RING(w_cursor, r->capacity) != r_pos)
		{
			ret = r->datas[r_pos];
			r->read_cursor++;
			break;
		}

		muggle_condition_variable_wait(&r->cv, &r->mtx, NULL);
	} while (1);
	muggle_mutex_unlock(&r->mtx);

	return ret;
}


// write, wake and read callbacks
static fn_muggle_ring_buffer_write muggle_ring_buffer_write_functions[MUGGLE_RING_BUFFER_WRITE_MODE_MAX] = {
	muggle_ring_buffer_write_lock, // MUGGLE_RING_BUFFER_WRITE_MODE_LOCK 
	muggle_ring_buffer_write_single, // MUGGLE_RING_BUFFER_WRITE_MODE_SINGLE 
	muggle_ring_buffer_write_busy_loop // MUGGLE_RING_BUFFER_WRITE_MODE_BUSY_LOOP 
};

static fn_muggle_ring_buffer_wake muggle_ring_buffer_wake_functions[MUGGLE_RING_BUFFER_READ_MODE_MAX] = {
	muggle_ring_buffer_wake_wait, // MUGGLE_RING_BUFFER_READ_MODE_WAIT 
	muggle_ring_buffer_wake_single_wait, // MUGGLE_RING_BUFFER_READ_MODE_SINGLE_WAIT 
	muggle_ring_buffer_wake_busy_loop, // MUGGLE_RING_BUFFER_READ_MODE_BUSY_LOOP 
	muggle_ring_buffer_wake_lock, // MUGGLE_RING_BUFFER_READ_MODE_LOCK 
};

static fn_muggle_ring_buffer_read muggle_ring_buffer_read_functions[MUGGLE_RING_BUFFER_READ_MODE_MAX] = {
	muggle_ring_buffer_read_wait, // MUGGLE_RING_BUFFER_READ_MODE_WAIT 
	muggle_ring_buffer_read_wait, // MUGGLE_RING_BUFFER_READ_MODE_SINGLE_WAIT 
	muggle_ring_buffer_read_busy_loop, // MUGGLE_RING_BUFFER_READ_MODE_BUSY_LOOP 
	muggle_ring_buffer_read_lock, // MUGGLE_RING_BUFFER_READ_MODE_LOCK 
};

int muggle_ring_buffer_init(muggle_ring_buffer_t *r, muggle_atomic_int capacity, int flag)
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

	r->next = 0;
	r->cursor = 0;
	r->read_cursor = 0;

	ret = muggle_mutex_init(&r->mtx);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	ret = muggle_condition_variable_init(&r->cv);
	if (ret != MUGGLE_OK)
	{
		muggle_mutex_destroy(&r->mtx);
		return ret;
	}

	r->datas = (void**)malloc(sizeof(void*) * r->capacity);
	if (r->datas == NULL)
	{
		muggle_mutex_destroy(&r->mtx);
		muggle_condition_variable_destroy(&r->cv);
		return MUGGLE_ERR_MEM_ALLOC;
	}

	return MUGGLE_OK;
}

int muggle_ring_buffer_destroy(muggle_ring_buffer_t *r)
{
	free(r->datas);
	muggle_mutex_destroy(&r->mtx);
	muggle_condition_variable_destroy(&r->cv);
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

void* muggle_ring_buffer_read(muggle_ring_buffer_t *r, muggle_atomic_int idx)
{
	return (*muggle_ring_buffer_read_functions[r->read_mode])(r, idx);
}

#endif /* ifndef MUGGLE_SUPPORT_FUTEX */
