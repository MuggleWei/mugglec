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

#define IDX_IN_POW_OF_2_RING(idx, capacity) ((idx) & ((capacity) - 1))

int muggle_ringbuffer_init(muggle_ringbuffer_t *r, muggle_atomic_int capacity)
{
	memset(r, 0, sizeof(muggle_ringbuffer_t));
	if (capacity <= 0)
	{
		return eMuggleErrInvalidParam;
	}

	r->capacity = (muggle_atomic_int)next_pow_of_2((uint64_t)capacity);
	if (r->capacity <= 0)
	{
		return eMuggleErrInvalidParam;
	}
	r->next = 0;
	r->cursor = 0;
	r->datas = (void**)malloc(sizeof(void*) * r->capacity);

	return eMuggleOk;
}

int muggle_ringbuffer_destroy(muggle_ringbuffer_t *r)
{
	free(r->datas);
	return eMuggleOk;
}

int muggle_ringbuffer_push(muggle_ringbuffer_t *r, void *data)
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
		expected = idx;
		desired = idx + 1;
	}

	// weakup
	muggle_futex_wake(&r->cursor);

	return eMuggleOk;
}

int muggle_ringbuffer_st_push(muggle_ringbuffer_t *r, void *data)
{
	// assignment
	r->datas[IDX_IN_POW_OF_2_RING(r->cursor, r->capacity)] = data;

	// move cursor
	muggle_atomic_store(&r->cursor, r->cursor+1, muggle_memory_order_release);

	// wakeup
	muggle_futex_wake(&r->cursor);

	return eMuggleOk;
}

void* muggle_ringbuffer_get(muggle_ringbuffer_t *r, muggle_atomic_int pos)
{
	pos = IDX_IN_POW_OF_2_RING(pos, r->capacity);
	muggle_atomic_int curr_pos;
	do {
		curr_pos = muggle_atomic_load(&r->cursor, muggle_memory_order_consume);
		if (IDX_IN_POW_OF_2_RING(curr_pos, r->capacity) != pos)
		{
			return r->datas[pos];
		}
		muggle_futex_wait(&r->cursor, pos);
	} while (1);

	return NULL;
}

void* muggle_ringbuffer_get_with_cache(
	muggle_ringbuffer_t *r,
	muggle_atomic_int pos,
	muggle_atomic_int *cursor_cache
)
{
	pos = IDX_IN_POW_OF_2_RING(pos, r->capacity);
	if (pos != *cursor_cache)
	{
		return r->datas[pos];
	}

	do {
		*cursor_cache = muggle_atomic_load(&r->cursor, muggle_memory_order_consume);
		*cursor_cache = IDX_IN_POW_OF_2_RING(*cursor_cache, r->capacity);
		if (pos != *cursor_cache)
		{
			return r->datas[pos];
		}
		muggle_futex_wait(&r->cursor, pos);
	} while (1);

	return NULL;
}
