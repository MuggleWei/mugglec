/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_CPP_MEMORY_POOL_H_
#define MUGGLE_CPP_MEMORY_POOL_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/memory/memory_pool.h"

NS_MUGGLE_BEGIN

class MUGGLE_CC_EXPORT MemoryPool
{
public:
	MemoryPool(unsigned int init_capacity, unsigned int block_size);
	~MemoryPool();

	MemoryPool(const MemoryPool&) = delete;
	MemoryPool(const MemoryPool&&) = delete;
	MemoryPool& operator=(const MemoryPool&) = delete;
	MemoryPool& operator=(const MemoryPool&&) = delete;

	void* alloc();
	void recycle(void *p);

	unsigned int capacity();
	unsigned int blockSize();
	unsigned int inUsedNum();

private:
	void* data_bufs_;				// data buffer array
	void** ptr_buf_;				// pointer buffer

	unsigned int top_;				// the index of the pointer buffer stack's top + 1

	unsigned int capacity_;			// current memory pool capacity
	unsigned int block_size_;		// size of single block

#if MUGGLE_DEBUG
	unsigned int peak_;				// record max number of block in use (Inaccurate)
#endif
};

NS_MUGGLE_END

#endif
