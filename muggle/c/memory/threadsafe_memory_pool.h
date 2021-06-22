/******************************************************************************
 *  @file         threadsafe_memory_pool.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec thread safe memory pool
 *****************************************************************************/
 
#ifndef MUGGLE_C_THREADSAFE_MEMORY_POOL_H_
#define MUGGLE_C_THREADSAFE_MEMORY_POOL_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"
#include "muggle/c/sync/mutex.h"

EXTERN_C_BEGIN

struct muggle_ts_memory_pool;

/**
 * @brief thread safe memory pool head
 */
typedef struct muggle_ts_memory_pool_head
{
	MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	struct muggle_ts_memory_pool *pool;
}muggle_ts_memory_pool_head_t;

/**
 * @brief thread safe memory pool head container
 */
typedef struct muggle_ts_memory_pool_head_ptr
{
	MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	muggle_ts_memory_pool_head_t *ptr;
}muggle_ts_memory_pool_head_ptr_t;

/**
 * @brief thread safe memory pool
 */
typedef struct muggle_ts_memory_pool
{
	muggle_atomic_int                capacity;
	muggle_atomic_int                block_size;
	void                             *data;
	muggle_ts_memory_pool_head_ptr_t *ptrs;

	MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	muggle_atomic_int alloc_cursor;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(1);
	muggle_atomic_int free_cursor;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(2);
	muggle_mutex_t free_mutex;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(3);
}muggle_ts_memory_pool_t;

/**
 * @brief init muggle thread safe memory pool
 *
 * @param pool       pointer to ts_memory_pool
 * @param capacity   expected capacity of pool
 * @param data_size  user data size
 *
 * @return
 *     - return 0 on success
 *     - otherwise failed and return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_ts_memory_pool_init(muggle_ts_memory_pool_t *pool, muggle_atomic_int capacity, muggle_atomic_int data_size);

/**
 * @brief destroy thread safe memory pool
 *
 * @param pool  pointer to ts_memory_pool
 */
MUGGLE_C_EXPORT
void muggle_ts_memory_pool_destroy(muggle_ts_memory_pool_t *pool);

/**
 * @brief thread safe memory pool allocate data
 *
 * @param pool  pointer to ts_memory_pool
 *
 * @return on success return data that allocated, if failed, return NULL
 */
MUGGLE_C_EXPORT
void* muggle_ts_memory_pool_alloc(muggle_ts_memory_pool_t *pool);

/**
 * @brief thread safe memory pool recycle data
 *
 * @param data  data allocated by thread safe memory pool
 */
MUGGLE_C_EXPORT
void muggle_ts_memory_pool_free(void *data);

EXTERN_C_END

#endif
