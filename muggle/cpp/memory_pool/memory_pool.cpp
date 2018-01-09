/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/cpp/memory_pool/memory_pool.h"
#include <exception>
#include <new>

NS_MUGGLE_BEGIN

MemoryPool::MemoryPool(unsigned int init_capacity, unsigned int block_size)
{
	if (!MuggleMemoryPoolInit(&pool_, init_capacity, block_size))
	{
		throw std::bad_alloc();
	}
}
MemoryPool::~MemoryPool()
{
	MuggleMemoryPoolDestroy(&pool_);
}

void* MemoryPool::alloc()
{
	return MuggleMemoryPoolAlloc(&pool_);
}
void MemoryPool::recycle(void *p)
{
	MuggleMemoryPoolFree(&pool_, p);
}

bool MemoryPool::EnsureSpace(unsigned int capacity)
{
	return MuggleMemoryPoolEnsureSpace(&pool_, capacity);
}

void MemoryPool::setConstantCapacity(bool flag)
{
	if (flag)
	{
		MuggleMemoryPoolSetFlag(&pool_, pool_.flag | MUGGLE_MEMORY_POOL_CONSTANT_SIZE);
	}
	else
	{
		MuggleMemoryPoolSetFlag(&pool_, pool_.flag & ~(MUGGLE_MEMORY_POOL_CONSTANT_SIZE));
	}
}
bool MemoryPool::isConstantCapacity()
{
	return pool_.flag & MUGGLE_MEMORY_POOL_CONSTANT_SIZE;
}

unsigned int MemoryPool::capacity()
{
	return pool_.capacity;
}
unsigned int MemoryPool::blockSize()
{
	return pool_.block_size;
}
unsigned int MemoryPool::inUsedNum()
{
	return pool_.used;
}

NS_MUGGLE_END
