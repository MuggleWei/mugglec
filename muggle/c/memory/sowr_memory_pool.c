/******************************************************************************
 *  @file         sowr_memory_pool.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sowr memory pool
 *****************************************************************************/
 
#include "sowr_memory_pool.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"

int muggle_sowr_memory_pool_init(muggle_sowr_memory_pool_t *pool, muggle_sync_t capacity, muggle_sync_t data_size)
{
	memset(pool, 0, sizeof(muggle_sowr_memory_pool_t));
	if (capacity <= 0)
	{
		capacity = 8;
	}

	capacity = (muggle_sync_t)next_pow_of_2((uint64_t)capacity);
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}
	pool->capacity = capacity;
	pool->block_size = (muggle_sync_t)next_pow_of_2((uint64_t)(data_size + sizeof(muggle_sowr_block_head_t)));
	pool->blocks = malloc(pool->block_size * pool->capacity);
	pool->alloc_idx = 0;
	pool->free_idx = 0;
	pool->cached_free_pos = pool->capacity - 1;

	for (muggle_sync_t i = 0; i < capacity; ++i)
	{
		muggle_sowr_block_head_t *block = (muggle_sowr_block_head_t*)((char*)pool->blocks + pool->block_size * i);
		block->pool = pool;
		block->block_idx = i;
	}

	return MUGGLE_OK;
}

void muggle_sowr_memory_pool_destroy(muggle_sowr_memory_pool_t *pool)
{
	free(pool->blocks);
}

void* muggle_sowr_memory_pool_alloc(muggle_sowr_memory_pool_t *pool)
{
	muggle_sync_t alloc_pos = IDX_IN_POW_OF_2_RING(pool->alloc_idx, pool->capacity);
	if (alloc_pos != pool->cached_free_pos)
	{
		muggle_sowr_block_head_t *block = (muggle_sowr_block_head_t*)((char*)pool->blocks + pool->block_size * alloc_pos);
		++pool->alloc_idx;
		return (void*)(block + 1);
	}

	pool->cached_free_pos = muggle_atomic_load(&pool->free_idx, muggle_memory_order_relaxed);
	pool->cached_free_pos -= 1;
	pool->cached_free_pos = IDX_IN_POW_OF_2_RING(pool->cached_free_pos, pool->capacity);
	if (alloc_pos != pool->cached_free_pos)
	{
		muggle_sowr_block_head_t *block = (muggle_sowr_block_head_t*)((char*)pool->blocks + pool->block_size * alloc_pos);
		++pool->alloc_idx;
		return (void*)(block + 1);
	}

	return NULL;
}

void muggle_sowr_memory_pool_free(void *data)
{
	muggle_sowr_block_head_t *block = (muggle_sowr_block_head_t*)data - 1;
	muggle_sowr_memory_pool_t *pool = block->pool;
	muggle_atomic_store(&pool->free_idx, block->block_idx + 1, muggle_memory_order_relaxed);
}


int muggle_sowr_memory_pool_is_all_free(muggle_sowr_memory_pool_t *pool)
{
	muggle_sync_t free_idx = muggle_atomic_load(&pool->free_idx, muggle_memory_order_relaxed);
	muggle_sync_t free_pos = IDX_IN_POW_OF_2_RING(free_idx, pool->capacity);
	muggle_sync_t alloc_pos = IDX_IN_POW_OF_2_RING(pool->alloc_idx, pool->capacity);

	return free_pos == alloc_pos ? 1 : 0;
}
