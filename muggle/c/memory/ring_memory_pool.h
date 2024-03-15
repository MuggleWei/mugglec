/******************************************************************************
 *  @file         ring_memory_poo.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-03-15
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec ring memory pool
 *****************************************************************************/

#ifndef MUGGLE_C_RING_MEMORY_POOL_H_
#define MUGGLE_C_RING_MEMORY_POOL_H_

#include "muggle/c/base/atomic.h"
#include "muggle/c/sync/spinlock.h"
#include "muggle/c/sync/sync_obj.h"

EXTERN_C_BEGIN

struct muggle_ring_memory_pool_tag;

/**
 * @brief ring memory pool block head
 */
typedef struct muggle_ring_mpool_block_tag {
	unsigned char cache_line[MUGGLE_CACHE_LINE_SIZE * 2];
	struct muggle_ring_memory_pool_tag *pool; //!< belong to pool
	muggle_atomic_int32 in_use; //!< is in used
	uint32_t block_idx;  //!< block index
} muggle_ring_mpool_block_head_t;

/**
 * @brief ring memory pool
 */
typedef struct muggle_ring_memory_pool_tag {
	void *blocks; //!< blocks
	muggle_spinlock_t write_spinlock; //!< write lock
	muggle_sync_t capacity; //!< capacity of pool
	muggle_sync_t block_size; //!< block size
	muggle_sync_t alloc_idx; //!< allocate cursor
} muggle_ring_memory_pool_t;

/**
 * @brief initialize ring memory pool
 *
 * NOTE: init capacity is not real capacity, actual capacity is pow of 2
 *
 * @param pool       ring memory pool
 * @param capacity   init capacity
 * @param data_size  memory data size
 *
 * @return
 *     - on success, return 0
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_ring_memory_pool_init(muggle_ring_memory_pool_t *pool,
								 muggle_sync_t capacity,
								 muggle_sync_t data_size);

/**
 * @brief destroy ring memory pool
 *
 * @param pool  ring memory pool pointer
 */
MUGGLE_C_EXPORT
void muggle_ring_memory_pool_destroy(muggle_ring_memory_pool_t *pool);

/**
 * @brief ring memory pool allocate data
 *
 * @NOTE:
 *     it's not thread safe, user need guarantee mutex with other thread or use
 *     muggle_ring_memory_pool_thread_safe_alloc function
 *
 * @param pool  ring memory pool pointer
 *
 * @return new data space
 */
MUGGLE_C_EXPORT
void *muggle_ring_memory_pool_alloc(muggle_ring_memory_pool_t *pool);

/**
 * @brief ring memory pool allocate data
 *
 * @NOTE: This function is thread safe
 *
 * @param pool  ring memory pool pointer
 *
 * @return new data space
 */
MUGGLE_C_EXPORT
void *muggle_ring_memory_pool_threadsafe_alloc(muggle_ring_memory_pool_t *pool);

/**
 * @brief recycle data
 *
 * @param data  data allocated by a ring memory pool
 */
MUGGLE_C_EXPORT
void muggle_ring_memory_pool_free(void *data);

EXTERN_C_END

#endif // !MUGGLE_C_RING_MEMORY_POOL_H_
