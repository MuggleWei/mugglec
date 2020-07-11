/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "threadsafe_memory_pool.h"
#include <stdlib.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/sync/futex.h"

int muggle_ts_memory_pool_init(muggle_ts_memory_pool_t *pool, muggle_atomic_int capacity, muggle_atomic_int data_size)
{
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (data_size <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	capacity = (muggle_atomic_int)next_pow_of_2((uint64_t)capacity);
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	muggle_atomic_int block_size =
		(muggle_atomic_int)next_pow_of_2((uint64_t)(sizeof(muggle_ts_memory_pool_head_t) + data_size));
	if (block_size <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	pool->capacity = capacity;
	pool->block_size = block_size;
	pool->data = malloc(capacity * block_size);
	pool->ptrs = (muggle_ts_memory_pool_head_ptr_t*)malloc(capacity * sizeof(muggle_ts_memory_pool_head_ptr_t));
	pool->alloc_cursor = 0;
	pool->free_cursor = capacity;
	pool->free_fetch = pool->free_cursor;

	if (pool->data == NULL || pool->ptrs == NULL)
	{
		if (pool->data)
		{
			free(pool->data);
		}

		if (pool->ptrs)
		{
			free(pool->ptrs);
		}

		return MUGGLE_ERR_MEM_ALLOC;
	}

	for (muggle_atomic_int i = 0; i < capacity; i++)
	{
		muggle_ts_memory_pool_head_t *block =
			(muggle_ts_memory_pool_head_t*)((char*)pool->data + block_size * i);
		block->pool = pool;
		pool->ptrs[i].ptr = block;
	}

	return MUGGLE_OK;
}

void muggle_ts_memory_pool_destroy(muggle_ts_memory_pool_t *pool)
{
	if (pool->data)
	{
		free(pool->data);
		pool->data = NULL;
	}

	if (pool->ptrs)
	{
		free(pool->ptrs);
		pool->ptrs = NULL;
	}
}

void* muggle_ts_memory_pool_alloc(muggle_ts_memory_pool_t *pool)
{
	muggle_atomic_int expected = pool->alloc_cursor;
	muggle_atomic_int alloc_cursor = 0;
	muggle_atomic_int alloc_pos = 0;
	void *data = NULL;
	do {
		alloc_cursor = expected;
		if (alloc_cursor == muggle_atomic_load(&pool->free_cursor, muggle_memory_order_acquire))
		{
			return NULL;
		}

		// fetch data
		// NOTE: fetch data must before move cursor, avoid error this thread pending and at the same 
		// time other thread alloc and free, then this thread wake
		alloc_pos = IDX_IN_POW_OF_2_RING(alloc_cursor, pool->capacity);
		data = (void*)(pool->ptrs[alloc_pos].ptr + 1);

		// move allocate cursor
	} while (!muggle_atomic_cmp_exch_weak(&pool->alloc_cursor, &expected, alloc_cursor + 1, muggle_memory_order_relaxed)
			&& expected != alloc_pos);

	return data;
}

void muggle_ts_memory_pool_free(void *data)
{
	muggle_ts_memory_pool_head_t *block = (muggle_ts_memory_pool_head_t*)data - 1;
	muggle_ts_memory_pool_t *pool = block->pool;

	// free data
	muggle_atomic_int fetch_cursor = muggle_atomic_fetch_add(&pool->free_fetch, 1, muggle_memory_order_relaxed);
	muggle_atomic_int free_pos = IDX_IN_POW_OF_2_RING(fetch_cursor, pool->capacity);
	pool->ptrs[free_pos].ptr = block;

	// move free cursor
	muggle_atomic_int expected = fetch_cursor;
	while (!muggle_atomic_cmp_exch_weak(&pool->free_cursor, &expected, fetch_cursor + 1, muggle_memory_order_release)
			&& expected != fetch_cursor)
	{
		muggle_futex_wait(&pool->free_cursor, expected, NULL);
		expected = fetch_cursor;
	}
	muggle_futex_wake_all(&pool->free_cursor);
}
