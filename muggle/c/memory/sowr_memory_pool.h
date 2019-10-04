/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_SOWR_MEMORY_POOL_H_
#define MUGGLE_C_SOWR_MEMORY_POOL_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <pthread.h>
#endif

EXTERN_C_BEGIN

// sowr - sequential only one writer and only one reader 
// allocate A happen before allocate B, if free A, must happen before free B
// allocate b1, b2, b3, b4, b5 and free b1, b2, b3, b4, b5, it's ok
// allocate b1, b2, b3, b4, b5 and only free b5, it's ok too, it's mean free b5 and all blocks allocate before b5

struct muggle_sowr_memory_pool_tag;

typedef struct muggle_sowr_block_head_tag
{
	struct muggle_sowr_memory_pool_tag *pool;
	int block_idx;
	int alignment_padding;
}muggle_sowr_block_head_t;

typedef struct muggle_sowr_memory_pool_tag
{
	void *blocks;
	muggle_atomic_int capacity;
	muggle_atomic_int block_size;
	muggle_atomic_int alloc_idx;
	muggle_atomic_int free_idx;
	muggle_atomic_int cached_free_pos;
}muggle_sowr_memory_pool_t;

MUGGLE_CC_EXPORT
int muggle_sowr_memory_pool_init(muggle_sowr_memory_pool_t *pool, muggle_atomic_int capacity, muggle_atomic_int data_size);

MUGGLE_CC_EXPORT
void muggle_sowr_memory_pool_destroy(muggle_sowr_memory_pool_t *pool);

MUGGLE_CC_EXPORT
void* muggle_sowr_memory_pool_alloc(muggle_sowr_memory_pool_t *pool);

MUGGLE_CC_EXPORT
void muggle_sowr_memory_pool_free(void *data);

MUGGLE_CC_EXPORT
int muggle_sowr_memory_pool_is_all_free(muggle_sowr_memory_pool_t *pool);

EXTERN_C_END

#endif
