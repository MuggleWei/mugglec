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
	if (r->flag & MUGGLE_RINGBUFFER_FLAG_SINGLE_WRITER)
	{
		r->write_mode = MUGGLE_RINGBUFFER_WRITE_MODE_SINGLE;
	}
	else if (r->flag & MUGGLE_RINGBUFFER_FLAG_WNUM_GT_CPU)
	{
		r->write_mode = MUGGLE_RINGBUFFER_WRITE_MODE_LOCK;
	}
	else
	{
		r->write_mode = MUGGLE_RINGBUFFER_WRITE_MODE_BUSY_LOOP;
	}
	r->next = 0;
	r->cursor = 0;
	r->datas = (void**)malloc(sizeof(void*) * r->capacity);

	muggle_mutex_init(&r->write_mutex);

	return MUGGLE_OK;
}

int muggle_ringbuffer_destroy(muggle_ringbuffer_t *r)
{
	free(r->datas);
	muggle_mutex_destroy(&r->write_mutex);
	return MUGGLE_OK;
}

int muggle_ringbuffer_write(muggle_ringbuffer_t *r, void *data)
{
	switch (r->write_mode)
	{
		case MUGGLE_RINGBUFFER_WRITE_MODE_SINGLE:
		{
			// assignment
			r->datas[IDX_IN_POW_OF_2_RING(r->cursor, r->capacity)] = data;

			// move cursor
			muggle_atomic_store(&r->cursor, r->cursor+1, muggle_memory_order_release);
		}break;
		case MUGGLE_RINGBUFFER_WRITE_MODE_BUSY_LOOP:
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
		}break;
		default:
		{
			muggle_mutex_lock(&r->write_mutex);

			// assignment
			r->datas[IDX_IN_POW_OF_2_RING(r->cursor, r->capacity)] = data;

			// move cursor
			muggle_atomic_store(&r->cursor, r->cursor+1, muggle_memory_order_release);

			muggle_mutex_unlock(&r->write_mutex);
		}break;
	}

	if (r->flag & MUGGLE_RINGBUFFER_FLAG_SINGLE_READER)
	{
		muggle_futex_wake_one(&r->cursor);
	}
	else
	{
		muggle_futex_wake_all(&r->cursor);
	}

	return MUGGLE_OK;
}

void* muggle_ringbuffer_read(muggle_ringbuffer_t *r, muggle_atomic_int pos)
{
	pos = IDX_IN_POW_OF_2_RING(pos, r->capacity);
	muggle_atomic_int curr_pos;
	do {
		curr_pos = muggle_atomic_load(&r->cursor, muggle_memory_order_acquire);
		if (IDX_IN_POW_OF_2_RING(curr_pos, r->capacity) != pos)
		{
			return r->datas[pos];
		}
		muggle_futex_wait(&r->cursor, pos, NULL);
	} while (1);

	return NULL;
}

