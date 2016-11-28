/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef __MUGGLE_MEMORY_POOL_H__
#define __MUGGLE_MEMORY_POOL_H__

#include "muggle/base/macro.h"
#include <stdbool.h>

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS && defined(MUGGLE_USE_DLLS)
	#ifdef MUGGLE_MEMORY_POOL_EXPORTS
		#define MUGGLE_MEMORY_POOL_EXPORT __declspec(dllexport)
	#else
		#define MUGGLE_MEMORY_POOL_EXPORT __declspec(dllimport)
	#endif
#else
	#define MUGGLE_MEMORY_POOL_EXPORT
#endif

// memory pool flag
#define MUGGLE_MEMORY_POOL_CONSTANT_SIZE	0x01

typedef struct MemoryPool_tag
{
	void**			memory_pool_data_bufs;  // data buffer array
	void**			memory_pool_ptr_buf;    // pointer buffer

	unsigned int	alloc_index;            // next time, alloc pointer index in pointer buffer
	unsigned int	free_index;             // next time, free pointer index in pointer buffer

	unsigned int	capacity;               // current memory pool capacity
	unsigned int	used;                   // how many block in use

	unsigned int	block_size;             // size of single block
	unsigned int	num_buf;                // the number of data buffer

	unsigned int	flag;					// flags

#if MUGGLE_DEBUG
	unsigned int	peak;                   // record max number of block in use
#endif
}MemoryPool;

MUGGLE_MEMORY_POOL_EXPORT 
bool MemoryPoolInit(MemoryPool* pool, unsigned int init_capacity, unsigned int block_size);

MUGGLE_MEMORY_POOL_EXPORT 
void MemoryPoolDestroy(MemoryPool* pool);

MUGGLE_MEMORY_POOL_EXPORT 
void* MemoryPoolAlloc(MemoryPool* pool);

MUGGLE_MEMORY_POOL_EXPORT 
void MemoryPoolFree(MemoryPool* pool, void* p_data);

MUGGLE_MEMORY_POOL_EXPORT 
bool MemoryPoolEnsureSpace(MemoryPool* pool, unsigned int capacity);

MUGGLE_MEMORY_POOL_EXPORT
unsigned int MemoryPoolGetFlag(MemoryPool* pool);

MUGGLE_MEMORY_POOL_EXPORT
void MemoryPoolSetFlag(MemoryPool* pool, unsigned int flag);

EXTERN_C_END

#endif