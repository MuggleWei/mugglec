/******************************************************************************
 *  @file         sowr_memory_pool.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sowr memory pool
 *
 * sowr - sequential only one writer and only one reader 
 *
 * sowr is a special memory pool, user need guarantee only one writer and only one reader
 * use it in the same time. Can used to exchange data between two threads
 *
 * - allocate A happen before allocate B, if free A, must happen before free B
 * - allocate b1, b2, b3, b4, b5 and free b1, b2, b3, b4, b5, it's ok
 * - allocate b1, b2, b3, b4, b5 and only free b5, it's ok too, it's mean free b5 and all blocks allocate before b5
 *****************************************************************************/
 
#ifndef MUGGLE_C_SOWR_MEMORY_POOL_H_
#define MUGGLE_C_SOWR_MEMORY_POOL_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"
#include "muggle/c/sync/sync_obj.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <pthread.h>
#endif

EXTERN_C_BEGIN


struct muggle_sowr_memory_pool_tag;

/**
 * @brief sowr memory pool block head
 */
typedef struct muggle_sowr_block_head_tag
{
	struct muggle_sowr_memory_pool_tag *pool;
	int block_idx;
	int alignment_padding;
}muggle_sowr_block_head_t;

/**
 * @brief sowr memory pool
 */
typedef struct muggle_sowr_memory_pool_tag
{
	void *blocks;
	muggle_sync_t capacity;
	muggle_sync_t block_size;
	muggle_sync_t alloc_idx;
	muggle_sync_t free_idx;
	muggle_sync_t cached_free_pos;
}muggle_sowr_memory_pool_t;

/**
 * @brief initialize sowr memory pool
 *
 * NOTE: init capacity is not real capacity, actual capacity is pow of 2
 *
 * @param pool       sowr memory pool pointer
 * @param capacity   init capacity
 * @param data_size  memory data size
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_sowr_memory_pool_init(muggle_sowr_memory_pool_t *pool, muggle_sync_t capacity, muggle_sync_t data_size);

/**
 * @brief destroy sowr memory pool
 *
 * @param pool   sowr memory pool pointer
 */
MUGGLE_C_EXPORT
void muggle_sowr_memory_pool_destroy(muggle_sowr_memory_pool_t *pool);

/**
 * @brief sowr memory allocate data
 *
 * @param pool   sowr memory pool pointer
 *
 * @return new data space
 */
MUGGLE_C_EXPORT
void* muggle_sowr_memory_pool_alloc(muggle_sowr_memory_pool_t *pool);

/**
 * @brief recycle data
 *
 * @param data  data allocated by a sowr memory pool
 */
MUGGLE_C_EXPORT
void muggle_sowr_memory_pool_free(void *data);

/**
 * @brief detect all data that allocated from sowr memory pool are recycle
 *
 * @param pool   sowr memory pool pointer
 *
 * @return
 *     - return 1 represent all data has been recycled
 *     - return 0 represent there is data that has not been recycled
 */
MUGGLE_C_EXPORT
int muggle_sowr_memory_pool_is_all_free(muggle_sowr_memory_pool_t *pool);

EXTERN_C_END

#endif
