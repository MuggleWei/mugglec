/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "ringbuffer.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/sync/futex.h"

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
	r->next = 0;
	r->cursor = 0;
	r->datas = (void**)malloc(sizeof(void*) * r->capacity);

	return MUGGLE_OK;
}

int muggle_ringbuffer_destroy(muggle_ringbuffer_t *r)
{
	free(r->datas);
	return MUGGLE_OK;
}

int muggle_ringbuffer_write(muggle_ringbuffer_t *r, void *data)
{
	if (r->flag & MUGGLE_RINGBUFFER_FLAG_SINGLE_WRITER) 
	{
		// assignment
		r->datas[IDX_IN_POW_OF_2_RING(r->cursor, r->capacity)] = data;

		// move cursor
		muggle_atomic_store(&r->cursor, r->cursor+1, muggle_memory_order_release);

		// wakeup
		if (r->flag & MUGGLE_RINGBUFFER_FLAG_SINGLE_READER)
		{
			muggle_futex_wake_one(&r->cursor);
		}
		else
		{
			muggle_futex_wake_all(&r->cursor);
		}
	}
	else
	{
		// move next
		muggle_atomic_int idx = muggle_atomic_fetch_add(&r->next, 1, muggle_memory_order_relaxed);

		// assignment
		r->datas[IDX_IN_POW_OF_2_RING(idx, r->capacity)] = data;

		// move cursor
		muggle_atomic_int expected = idx;
		muggle_atomic_int desired = idx + 1;
		while (!muggle_atomic_cmp_exch_weak(&r->cursor, &expected, desired, muggle_memory_order_acq_rel))
		{
			if (r->flag & MUGGLE_RINGBUFFER_FLAG_WNUM_GT_CPU)
			{
				if (expected == idx)
				{
					// fail spuriously
					continue;
				}
				muggle_futex_wait(&r->cursor, expected, NULL);
			}
			expected = idx;
			desired = idx + 1;
		}

		if ((r->flag & MUGGLE_RINGBUFFER_FLAG_SINGLE_READER) && 
			!(r->flag & MUGGLE_RINGBUFFER_FLAG_WNUM_GT_CPU))
		{
			muggle_futex_wake_one(&r->cursor);
		}
		else
		{
			muggle_futex_wake_all(&r->cursor);
		}
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

