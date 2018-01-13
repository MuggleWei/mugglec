/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_CPP_THREAD_SAFE_MEMORY_POOL_H_
#define MUGGLE_CPP_THREAD_SAFE_MEMORY_POOL_H_

#include "muggle/c/base/macro.h"
#include <atomic>

NS_MUGGLE_BEGIN

class ThreadSafeMemoryPool
{
public:
	MUGGLE_CC_EXPORT ThreadSafeMemoryPool(unsigned int init_capacity, unsigned int block_size);
	MUGGLE_CC_EXPORT ~ThreadSafeMemoryPool();

	MUGGLE_CC_EXPORT void* alloc();
	MUGGLE_CC_EXPORT void recycle(void *p);

	MUGGLE_CC_EXPORT unsigned int capacity();
	MUGGLE_CC_EXPORT unsigned int blockSize();
	MUGGLE_CC_EXPORT unsigned int inUsedNum();

private:
	struct Node
	{
		void* next;
		void* data()
		{
			return (void*)((char*)(&next) + sizeof(void*));
		}
	};

private:
	std::atomic<Node*> head_;		// node stack's head
	void *data_buf_;				// the whole pool's memory space

	std::atomic_uint used_;			// how many block in use

	unsigned int capacity_;			// current memory pool capacity
	unsigned int block_size_;		// size of single block

#if MUGGLE_DEBUG
	unsigned int peak_;				// record max number of block in use (Inaccurate)
#endif
};

NS_MUGGLE_END

#endif