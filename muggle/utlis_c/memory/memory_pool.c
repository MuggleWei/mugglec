/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base_c/memory_pool.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool MemoryPoolInit(MemoryPool* pool, unsigned int init_capacity, unsigned int block_size)
{
	memset(pool, 0, sizeof(MemoryPool));
	init_capacity = init_capacity == 0 ? 8 : init_capacity;

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
	pool->memory_pool_data_bufs[0] = (void*)malloc(block_size * init_capacity);
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

#if MUGGLE_DEBUG
	pool->peak = 0;
#endif

	void* ptr_buf = pool->memory_pool_data_bufs[0];
	unsigned int i;
	for (i = 0; i < init_capacity; ++i)
	{
		pool->memory_pool_ptr_buf[i] = (void*)((char*)ptr_buf + i * block_size);
	}

	return true;
}
void MemoryPoolDestroy(MemoryPool* pool)
{
	unsigned int i;
	for (i = 0; i<pool->num_buf; ++i)
	{
		free((void*)pool->memory_pool_data_bufs[i]);
	}
	free((void*)pool->memory_pool_data_bufs);
	free((void*)pool->memory_pool_ptr_buf);

	memset(pool, 0, sizeof(MemoryPool));
}
void* MemoryPoolAlloc(MemoryPool* pool)
{
	if (pool->used == pool->capacity)
	{
		if (!MemoryPoolEnsureSpace(pool, pool->capacity * 2))
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
void MemoryPoolFree(MemoryPool* pool, void* p_data)
{
	pool->memory_pool_ptr_buf[pool->free_index] = (void*)p_data;
	++pool->free_index;
	if (pool->free_index == pool->capacity)
	{
		pool->free_index = 0;
	}
	--pool->used;
}

bool MemoryPoolEnsureSpace(MemoryPool* pool, unsigned int capacity)
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
	unsigned int delta_size = capacity - pool->capacity;
	void** new_bufs = (void**)malloc(sizeof(void*) * (pool->num_buf + 1));
	if (new_bufs == NULL)
	{
		return false;
	}
	memcpy(new_bufs, pool->memory_pool_data_bufs, sizeof(void*) * pool->num_buf);
	new_bufs[pool->num_buf] = (void*)malloc(pool->block_size * delta_size);
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
	unsigned int num_free_section1 = 0, num_free_section2 = 0;
	unsigned int num_alloc_section1 = 0, num_alloc_section2 = 0;

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
	unsigned int offset = 0;
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
	if (num_free_section1 > 0)
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
	unsigned int i;
	for (i = 0; i < delta_size; ++i)
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

MUGGLE_BASE_C_EXPORT
unsigned int MemoryPoolGetFlag(MemoryPool* pool)
{
	return pool->flag;
}

MUGGLE_BASE_C_EXPORT
void MemoryPoolSetFlag(MemoryPool* pool, unsigned int flag)
{
	pool->flag = flag;
}