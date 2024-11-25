/******************************************************************************
 *  @file         threadsafe_memory_pool.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec thread safe memory pool
 *****************************************************************************/
 
#include "threadsafe_memory_pool.h"
#include <stdlib.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/base/atomic.h"

int muggle_ts_memory_pool_init(muggle_ts_memory_pool_t *pool, muggle_sync_t capacity, muggle_sync_t data_size)
{
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (data_size <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	capacity = (muggle_sync_t)next_pow_of_2((uint64_t)capacity);
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	// align block_size to MUGGLE_CACHE_LINE_X2_SIZE
	muggle_sync_t block_size = 
		(muggle_sync_t)sizeof(muggle_ts_memory_pool_head_t) + data_size;
	muggle_sync_t quotient = block_size / MUGGLE_CACHE_LINE_X2_SIZE;
	muggle_sync_t remainder = block_size % MUGGLE_CACHE_LINE_X2_SIZE;
	block_size = quotient * MUGGLE_CACHE_LINE_X2_SIZE +
		(remainder == 0 ? 0 : MUGGLE_CACHE_LINE_X2_SIZE);
	if (block_size <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	int ret = muggle_mutex_init(&pool->free_mutex);
	if (ret != 0)
	{
		return ret;
	}

	pool->capacity = capacity;
	pool->block_size = block_size;
#if MUGGLE_C_HAVE_ALIGNED_ALLOC
	pool->data = aligned_alloc(MUGGLE_CACHE_LINE_X2_SIZE, capacity * block_size);
	pool->ptrs = (muggle_ts_memory_pool_head_ptr_t*)aligned_alloc(
			MUGGLE_CACHE_LINE_X2_SIZE,
			capacity * sizeof(muggle_ts_memory_pool_head_ptr_t));
#else
	pool->data = malloc(capacity * block_size);
	pool->ptrs = (muggle_ts_memory_pool_head_ptr_t*)malloc(
			capacity * sizeof(muggle_ts_memory_pool_head_ptr_t));
#endif
	pool->alloc_cursor = 0;
	pool->free_cursor = capacity;

	if (pool->data == NULL || pool->ptrs == NULL)
	{
		muggle_mutex_destroy(&pool->free_mutex);

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

	for (muggle_sync_t i = 0; i < capacity; i++)
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
	muggle_mutex_destroy(&pool->free_mutex);

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
	muggle_sync_t expected = pool->alloc_cursor;
	muggle_sync_t alloc_cursor = 0;
	muggle_sync_t alloc_pos = 0;
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

	muggle_mutex_lock(&pool->free_mutex);

	muggle_sync_t free_pos = IDX_IN_POW_OF_2_RING(pool->free_cursor, pool->capacity);
	pool->ptrs[free_pos].ptr = block;

	// use atomic store for writer see correct order
	muggle_atomic_store(&pool->free_cursor, pool->free_cursor + 1, muggle_memory_order_release);

	muggle_mutex_unlock(&pool->free_mutex);
}
