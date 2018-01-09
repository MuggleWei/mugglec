/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_CPP_MEMORY_POOL_H_
#define MUGGLE_CPP_MEMORY_POOL_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/memory_pool/memory_pool.h"

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

	bool EnsureSpace(unsigned int capacity);

	void setConstantCapacity(bool flag);
	bool isConstantCapacity();

	unsigned int capacity();
	unsigned int blockSize();
	unsigned int inUsedNum();

private:
	MuggleMemoryPool pool_;
};

NS_MUGGLE_END

#endif