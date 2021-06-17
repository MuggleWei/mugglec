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
#include <stdbool.h>

EXTERN_C_BEGIN

// memory pool flag
#define MUGGLE_MEMORY_POOL_CONSTANT_SIZE	0x01  //!< memory pool use constant size

typedef struct muggle_memory_pool_tag
{
	void**			memory_pool_data_bufs;  //!< data buffer array
	void**			memory_pool_ptr_buf;    //!< pointer buffer

	unsigned int	alloc_index;            //!< next time, alloc pointer index in pointer buffer
	unsigned int	free_index;             //!< next time, free pointer index in pointer buffer

	unsigned int	capacity;               //!< current memory pool capacity
	unsigned int	used;                   //!< how many block in use

	unsigned int	block_size;             //!< size of single block
	unsigned int	num_buf;                //!< the number of data buffer

	unsigned int	flag;					//!< flags

#if MUGGLE_DEBUG
	unsigned int	peak;                   //!< record max number of block in use
#endif
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
bool muggle_memory_pool_init(muggle_memory_pool_t* pool, unsigned int init_capacity, unsigned int block_size);

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
bool muggle_memory_pool_ensure_space(muggle_memory_pool_t* pool, unsigned int capacity);

/**
 * @brief get memory pool flag
 *
 * @param pool  memory pool pointer
 *
 * @return memory pool's flag
 */
MUGGLE_C_EXPORT
unsigned int muggle_memory_pool_get_flag(muggle_memory_pool_t* pool);

/**
 * @brief set memory pool flag
 *
 * @param pool  memory pool pointer
 * @param flag  flag in MUGGLE_MEMORY_POOL_*
 */
MUGGLE_C_EXPORT
void muggle_memory_pool_set_flag(muggle_memory_pool_t* pool, unsigned int flag);

EXTERN_C_END

#endif
