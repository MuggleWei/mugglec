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

	capacity = (muggle_sync_t)muggle_next_pow_of_2((uint64_t)capacity);
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	muggle_sync_t block_size =
		(muggle_sync_t)sizeof(muggle_ts_memory_pool_head_t) + data_size;
	block_size = MUGGLE_ALIGN_TRUE_SHARING(block_size);
	if (block_size <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	pool->capacity = capacity;
	pool->block_size = block_size;

	size_t total_bytes = (size_t)capacity * (size_t)block_size;
#if MUGGLE_C_HAVE_ALIGNED_ALLOC
	pool->data = aligned_alloc(MUGGLE_CACHE_LINE_SIZE, total_bytes);
	pool->ptrs = (muggle_ts_memory_pool_head_ptr_t*)aligned_alloc(
			MUGGLE_CACHE_LINE_SIZE,
			capacity * sizeof(muggle_ts_memory_pool_head_ptr_t));
#else
	pool->data = malloc(total_bytes);
	pool->ptrs = (muggle_ts_memory_pool_head_ptr_t*)malloc(
			capacity * sizeof(muggle_ts_memory_pool_head_ptr_t));
#endif
	pool->alloc_idx = 0;
	pool->cached_free_pos = 0;
	pool->free_idx = 0;

	muggle_spinlock_init(&pool->free_spinlock);

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
	void *data = NULL;

	muggle_sync_t expected = pool->alloc_idx;
	muggle_sync_t alloc_pos = 0;
	do {
		alloc_pos = MUGGLE_IDX_IN_POW_OF_2_RING(expected + 1, pool->capacity);
		if (alloc_pos == pool->cached_free_pos) {
			pool->cached_free_pos =
				muggle_atomic_load(&pool->free_idx, muggle_memory_order_acquire);
			if (alloc_pos == pool->cached_free_pos) {
				return NULL;
			}
		}

		data = (void*)(pool->ptrs[expected].ptr + 1);
	} while (!muggle_atomic_cmp_exch_weak(&pool->alloc_idx, &expected, alloc_pos, muggle_memory_order_relaxed));

	return data;
}

void muggle_ts_memory_pool_free(void *data)
{
	muggle_ts_memory_pool_head_t *block =
		(muggle_ts_memory_pool_head_t*)data - 1;
	muggle_ts_memory_pool_t *pool = block->pool;

	muggle_spinlock_lock(&pool->free_spinlock);

	pool->ptrs[pool->free_idx].ptr = block;

	muggle_sync_t free_pos =
		MUGGLE_IDX_IN_POW_OF_2_RING(pool->free_idx + 1, pool->capacity);
	muggle_atomic_store(&pool->free_idx, free_pos, muggle_memory_order_release);

	muggle_spinlock_unlock(&pool->free_spinlock);
}
