/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "memory_pool.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MUGGLE_MEMPOOL_SIZE_2GB \
	((uint64_t)2llu * (uint64_t)1024llu * (uint64_t)1024llu * (uint64_t)1024llu)
#define MUGGLE_MEMPOOL_SIZE_4GB \
	((uint64_t)4llu * (uint64_t)1024llu * (uint64_t)1024llu * (uint64_t)1024llu)

bool muggle_memory_pool_init(muggle_memory_pool_t* pool, uint32_t init_capacity, uint32_t block_size)
{
	memset(pool, 0, sizeof(muggle_memory_pool_t));
	init_capacity = init_capacity == 0 ? 8 : init_capacity;
	if (block_size == 0)
	{
		return false;
	}

	pool->memory_pool_data_bufs = (void**)malloc(sizeof(void*));
	if (pool->memory_pool_data_bufs == NULL)
	{
		return false;
	}
	pool->memory_pool_ptr_buf = (void**)malloc(sizeof(void*) * init_capacity);
	if (pool->memory_pool_ptr_buf == NULL)
	{
		free(pool->memory_pool_data_bufs);
		pool->memory_pool_data_bufs = NULL;
		return false;
	}

	// avoid total_bytes >= 4GB in 32bit platform
	uint64_t total_bytes = (uint64_t)block_size * (uint64_t)init_capacity;
	if ((sizeof(size_t) < 8) && (total_bytes >= MUGGLE_MEMPOOL_SIZE_4GB))
	{
		return false;
	}

	pool->memory_pool_data_bufs[0] = (void*)malloc((size_t)total_bytes);
	if (pool->memory_pool_data_bufs[0] == NULL)
	{
		free(pool->memory_pool_data_bufs);
		pool->memory_pool_data_bufs = NULL;
		free(pool->memory_pool_ptr_buf);
		pool->memory_pool_ptr_buf = NULL;
		return false;
	}

	pool->alloc_index = pool->free_index = 0;
	pool->capacity = init_capacity;
	pool->used = 0;

	pool->block_size = block_size;
	pool->num_buf = 1;

	pool->flag = 0;

	// NOTE:
	// avoid allocate >= 4G of memory in muggle_memory_pool_ensure_space, it 
	// will lead bug in 32bit platform
	pool->max_delta_cap = MUGGLE_MEMPOOL_SIZE_2GB / block_size;

	pool->peak = 0;

	void* ptr_buf = pool->memory_pool_data_bufs[0];
	uint32_t i;
	for (i = 0; i < init_capacity; ++i)
	{
		pool->memory_pool_ptr_buf[i] = (void*)((char*)ptr_buf + i * block_size);
	}

	return true;
}
void muggle_memory_pool_destroy(muggle_memory_pool_t* pool)
{
	uint32_t i;
	for (i = 0; i < pool->num_buf; ++i)
	{
		free((void*)pool->memory_pool_data_bufs[i]);
	}
	free((void*)pool->memory_pool_data_bufs);
	free((void*)pool->memory_pool_ptr_buf);

	memset(pool, 0, sizeof(muggle_memory_pool_t));
}
void* muggle_memory_pool_alloc(muggle_memory_pool_t* pool)
{
	if (pool->used == pool->capacity)
	{
		uint32_t delta_cap = pool->capacity;
		if ((pool->max_delta_cap > 0) && (delta_cap > pool->max_delta_cap)) {
			delta_cap = pool->max_delta_cap;
		}
		uint32_t new_cap = pool->capacity + delta_cap;

		if (!muggle_memory_pool_ensure_space(pool, new_cap))
		{
			return NULL;
		}
	}
	++pool->used;
#if MUGGLE_DEBUG
	if (pool->used > pool->peak)
	{
		pool->peak = pool->used;
	}
#endif

	void* ret = pool->memory_pool_ptr_buf[pool->alloc_index];
	++pool->alloc_index;
	if (pool->alloc_index == pool->capacity)
	{
		pool->alloc_index = 0;
	}
	return ret;
}
void muggle_memory_pool_free(muggle_memory_pool_t* pool, void* p_data)
{
	pool->memory_pool_ptr_buf[pool->free_index] = (void*)p_data;
	++pool->free_index;
	if (pool->free_index == pool->capacity)
	{
		pool->free_index = 0;
	}
	--pool->used;
}

bool muggle_memory_pool_ensure_space(muggle_memory_pool_t* pool, uint32_t capacity)
{
	// already have enough capacity
	if (capacity <= pool->capacity)
	{
		return true;
	}

	// if this is constant size pool, refuse to allocate new memory
	if (pool->flag & MUGGLE_MEMORY_POOL_CONSTANT_SIZE)
	{
		return false;
	}

	// allocate new data buffer
	uint32_t delta_cap = capacity - pool->capacity;
	void** new_bufs = (void**)malloc(sizeof(void*) * (pool->num_buf + 1));
	if (new_bufs == NULL)
	{
		return false;
	}
	memcpy(new_bufs, pool->memory_pool_data_bufs, sizeof(void*) * pool->num_buf);

	size_t delta_bytes = (size_t)pool->block_size * (size_t)delta_cap;
	new_bufs[pool->num_buf] = (void*)malloc(delta_bytes);
	if (new_bufs[pool->num_buf] == NULL)
	{
		free(new_bufs);
		return false;
	}

	// allocate new pointer buffer
	void** new_ptr_buf = (void**)malloc(sizeof(void*) * capacity);
	if (new_ptr_buf == NULL)
	{
		free(new_bufs[pool->num_buf]);
		free(new_bufs);
		return false;
	}

	// free old data buffer and reset data buffer
	free((void*)pool->memory_pool_data_bufs);
	pool->memory_pool_data_bufs = new_bufs;

	// get alloc and free section
	void **free_section1 = NULL, **free_section2 = NULL;
	void **alloc_section1 = NULL, **alloc_section2 = NULL;
	uint32_t num_free_section1 = 0, num_free_section2 = 0;
	uint32_t num_alloc_section1 = 0, num_alloc_section2 = 0;

	if (pool->used == pool->capacity)
	{
		/*
		*                  fm
		*  [x] [x] [x] [x] [x] [x] [x] [x] [x] [x] [x] [x]
		*/
		assert(pool->alloc_index == pool->free_index);

		free_section1 = (void**)&pool->memory_pool_ptr_buf[pool->free_index];
		num_free_section1 = pool->capacity - pool->free_index;

		free_section2 = (void**)&pool->memory_pool_ptr_buf[0];
		num_free_section2 = pool->free_index;
	}
	else if (pool->alloc_index > pool->free_index)
	{
		/*
		*           f           m
		*  [0] [0] [x] [x] [x] [0] [0] [0] [0] [0] [0] [0]
		*/
		free_section1 = (void**)&pool->memory_pool_ptr_buf[pool->free_index];
		num_free_section1 = pool->alloc_index - pool->free_index;

		alloc_section1 = (void**)&pool->memory_pool_ptr_buf[pool->alloc_index];
		num_alloc_section1 = pool->capacity - pool->alloc_index;

		alloc_section2 = (void**)&pool->memory_pool_ptr_buf[0];
		num_alloc_section2 = pool->free_index;
	}
	else
	{
		/*
		*           m           f
		*  [x] [x] [0] [0] [0] [x] [x] [x] [x] [x] [x] [x]
		*
		*                      or
		*          fm
		*  [0] [0] [0] [0] [0] [0] [0] [0] [0] [0] [0] [0]
		*/
		free_section1 = (void**)&pool->memory_pool_ptr_buf[pool->free_index];
		num_free_section1 = pool->capacity - pool->free_index;

		free_section2 = (void**)&pool->memory_pool_ptr_buf[0];
		num_free_section2 = pool->alloc_index;

		alloc_section1 = (void**)&pool->memory_pool_ptr_buf[pool->alloc_index];
		num_alloc_section1 = pool->free_index - pool->alloc_index;
	}

	// copy free section
	uint32_t offset = 0;
	pool->free_index = 0;
	memcpy(&new_ptr_buf[offset], (void*)free_section1, sizeof(void*) * num_free_section1);
	offset += num_free_section1;
	if (num_free_section2 > 0)
	{
		memcpy(&new_ptr_buf[offset], (void*)free_section2, sizeof(void*) * num_free_section2);
		offset += num_free_section2;
	}

	// copy alloc section
	pool->alloc_index = offset;
	if (num_alloc_section1 > 0)
	{
		memcpy(&new_ptr_buf[offset], (void*)alloc_section1, sizeof(void*) * num_alloc_section1);
		offset += num_alloc_section1;
		if (num_alloc_section2 > 0)
		{
			memcpy(&new_ptr_buf[offset], (void*)alloc_section2, sizeof(void*) * num_alloc_section2);
			offset += num_alloc_section2;
		}
	}

	// init new section
	uint32_t i;
	for (i = 0; i < delta_cap; ++i)
	{
		new_ptr_buf[offset + i] = (void*)((char*)pool->memory_pool_data_bufs[pool->num_buf] + i * pool->block_size);
	}

	// free old pointer buffer and reset pointer buffer
	free(pool->memory_pool_ptr_buf);
	pool->memory_pool_ptr_buf = new_ptr_buf;

	// update pool data
	++pool->num_buf;
	pool->capacity = capacity;

	return true;
}

uint32_t muggle_memory_pool_get_flag(muggle_memory_pool_t* pool)
{
	return pool->flag;
}

void muggle_memory_pool_set_flag(muggle_memory_pool_t* pool, uint32_t flag)
{
	pool->flag = flag;
}

void muggle_memory_pool_set_max_delta_cap(muggle_memory_pool_t* pool, uint32_t max_delta_cap)
{
	pool->max_delta_cap = max_delta_cap;
}
