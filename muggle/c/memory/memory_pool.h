/******************************************************************************
 *  @file         memory_pool.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec memory pool
 *****************************************************************************/
 
#ifndef MUGGLE_C_MEMORY_POOL_H_
#define MUGGLE_C_MEMORY_POOL_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>
#include <stdbool.h>

EXTERN_C_BEGIN

// memory pool flag
#define MUGGLE_MEMORY_POOL_CONSTANT_SIZE	0x01  //!< memory pool use constant size
#define MUGGLE_MEMORY_POOL_THP              0x02  //!< memory pool use THP

typedef struct muggle_memory_pool_tag
{
	void** memory_pool_data_bufs;  //!< data buffer array
	void** memory_pool_ptr_buf;    //!< pointer buffer

	uint32_t alloc_index; //!< next time, alloc pointer index in pointer buffer
	uint32_t free_index;  //!< next time, free pointer index in pointer buffer

	uint32_t capacity;    //!< current memory pool capacity
	uint32_t used;        //!< how many block in use

	uint32_t block_size;  //!< size of single block
	uint32_t num_buf;     //!< the number of data buffer

	uint32_t flag;        //!< flags

	uint32_t max_delta_cap; //!< max auto increase capacity in allocate, if it's 0, no limit
	uint32_t peak;          //!< record max number of block in use (debug only)
}muggle_memory_pool_t;

/**
 * @brief initialize memory pool
 *
 * @param pool           memory pool pointer
 * @param init_capacity  init capacity of memory pool
 * @param block_size     data size of memory pool
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_memory_pool_init(muggle_memory_pool_t* pool, uint32_t init_capacity, uint32_t block_size);

/**
 * @brief initialize memory pool and use THP
 * 
 * @param pool           memory pool pointer
 * @param init_capacity  init capacity of memory pool
 * @param block_size     data size of memory pool
 *
 * @return boolean
 *
 * @NOTE  this is Linux only, in other platform, equivalent to muggle_memory_pool_init
 */
MUGGLE_C_EXPORT
bool muggle_memory_pool_init_thp(muggle_memory_pool_t *pool, uint32_t init_capacity, uint32_t block_size);

/**
 * @brief destroy memory pool
 *
 * @param pool  memory pool pointer
 */
MUGGLE_C_EXPORT
void muggle_memory_pool_destroy(muggle_memory_pool_t* pool);

/**
 * @brief memory pool allocate new data
 *
 * @param pool  memory pool pointer
 *
 * @return 
 *     - on success, return memory block
 *     - on failed, return NULL
 */
MUGGLE_C_EXPORT 
void* muggle_memory_pool_alloc(muggle_memory_pool_t* pool);

/**
 * @brief recycle memory pool data
 *
 * @param pool    memory pool pointer
 * @param p_data  data that need to be recycled
 */
MUGGLE_C_EXPORT 
void muggle_memory_pool_free(muggle_memory_pool_t* pool, void* p_data);

/**
 * @brief ensure memory pool has enough size
 *
 * @param pool      memory pool pointer
 * @param capacity  expected capacity
 *
 * @return 
 *     - return true, when has enough capacity or success allocate new capacity
 *     - return false, has no enough capactiy and failed allocate new capacity
 */
MUGGLE_C_EXPORT 
bool muggle_memory_pool_ensure_space(muggle_memory_pool_t* pool, uint32_t capacity);

/**
 * @brief get memory pool flag
 *
 * @param pool  memory pool pointer
 *
 * @return memory pool's flag
 */
MUGGLE_C_EXPORT
uint32_t muggle_memory_pool_get_flag(muggle_memory_pool_t* pool);

/**
 * @brief set memory pool flag
 *
 * @param pool  memory pool pointer
 * @param flag  flag in MUGGLE_MEMORY_POOL_*
 */
MUGGLE_C_EXPORT
void muggle_memory_pool_set_flag(muggle_memory_pool_t* pool, uint32_t flag);

/**
 * @brief set memory pool max auto increase capacity in allocate
 *
 * @param pool           memory pool pointer
 * @param max_delta_cap  max increase capacity
 */
MUGGLE_C_EXPORT
void muggle_memory_pool_set_max_delta_cap( muggle_memory_pool_t* pool, uint32_t max_delta_cap);

EXTERN_C_END

#endif
