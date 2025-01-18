#include "ring_memory_pool.h"
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/log/log.h"
#include <stdlib.h>
#include <string.h>

int muggle_ring_memory_pool_init(muggle_ring_memory_pool_t *pool,
								 muggle_sync_t capacity,
								 muggle_sync_t data_size)
{
	memset(pool, 0, sizeof(*pool));
	if (capacity < 2) {
		capacity = 2;
	}

	capacity = (muggle_sync_t)muggle_next_pow_of_2((muggle_sync_t)capacity);
	if (capacity < 2) {
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (data_size == 0) {
		return MUGGLE_ERR_INVALID_PARAM;
	}

	muggle_spinlock_init(&pool->write_spinlock);
	pool->capacity = capacity;
	pool->block_size =
		muggle_next_pow_of_2(data_size + sizeof(muggle_ring_mpool_block_head_t));
	pool->alloc_idx = 0;

	pool->blocks = malloc(pool->block_size * pool->capacity);
	if (pool->blocks == NULL) {
		return MUGGLE_ERR_MEM_ALLOC;
	}

	for (muggle_sync_t i = 0; i < capacity; ++i) {
		muggle_ring_mpool_block_head_t *block =
			(muggle_ring_mpool_block_head_t *)((char *)pool->blocks +
											   pool->block_size * i);
		memset(block, 0, pool->block_size);
		block->pool = pool;
		block->in_use = 0;
		block->block_idx = i;
	}

	return MUGGLE_OK;
}

void muggle_ring_memory_pool_destroy(muggle_ring_memory_pool_t *pool)
{
	free(pool->blocks);
}

void *muggle_ring_memory_pool_alloc(muggle_ring_memory_pool_t *pool)
{
	MUGGLE_ASSERT(pool->alloc_idx < pool->capacity);
	muggle_ring_mpool_block_head_t *block = NULL;
	do {
		block = (muggle_ring_mpool_block_head_t *)((char *)pool->blocks +
												   pool->block_size *
													   pool->alloc_idx);
		++pool->alloc_idx;
		pool->alloc_idx = IDX_IN_POW_OF_2_RING(pool->alloc_idx, pool->capacity);

		if (muggle_atomic_load(&block->in_use, muggle_memory_order_relaxed) ==
			0) {
			break;
		}
	} while (1);

	block->in_use = 1;
	return (void *)(block + 1);
}

void *muggle_ring_memory_pool_threadsafe_alloc(muggle_ring_memory_pool_t *pool)
{
	muggle_spinlock_lock(&pool->write_spinlock);
	void *block = muggle_ring_memory_pool_alloc(pool);
	muggle_spinlock_unlock(&pool->write_spinlock);
	return block;
}

void muggle_ring_memory_pool_free(void *data)
{
	muggle_ring_mpool_block_head_t *block =
		(muggle_ring_mpool_block_head_t *)data - 1;
	muggle_atomic_store(&block->in_use, 0, muggle_memory_order_relaxed);
}
