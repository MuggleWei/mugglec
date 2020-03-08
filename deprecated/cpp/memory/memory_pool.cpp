/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "memory_pool.h"
#include <stdlib.h>
#include <exception>
#include <new>

NS_MUGGLE_BEGIN

MemoryPool::MemoryPool(unsigned int init_capacity, unsigned int block_size)
	: data_bufs_(nullptr)
	, ptr_buf_(nullptr)
	, capacity_(0)
	, block_size_(0)
#if MUGGLE_DEBUG
	, peak_(0)
#endif
{
	capacity_ = init_capacity == 0 ? 8 : init_capacity;
	block_size_ = block_size;
	if (block_size_ == 0)
	{
		throw std::bad_alloc();
	}

	data_bufs_ = (void*)malloc(block_size * init_capacity);
	if (data_bufs_ == nullptr)
	{
		throw std::bad_alloc();
	}

	ptr_buf_ = (void**)malloc(sizeof(void*) * init_capacity);
	if (ptr_buf_ == nullptr)
	{
		throw std::bad_alloc();
	}

	for (unsigned int i = 0; i < init_capacity; ++i)
	{
		ptr_buf_[i] = ((char*)data_bufs_) + i * block_size_;
	}
	top_ = capacity_;
}
MemoryPool::~MemoryPool()
{
	if (data_bufs_)
	{
		free(data_bufs_);
	}
	if (ptr_buf_)
	{
		free(ptr_buf_);
	}
}

void* MemoryPool::alloc()
{
	if (top_ == 0)
	{
		return nullptr;
	}

	--top_;

#if MUGGLE_DEBUG
	if (capacity_ - top_ > peak_)
	{
		peak_ = capacity_ - top_;
	}
#endif

	return ptr_buf_[top_];
}
void MemoryPool::recycle(void *p)
{
	ptr_buf_[top_] = p;
	++top_;
}

unsigned int MemoryPool::capacity()
{
	return capacity_;
}
unsigned int MemoryPool::blockSize()
{
	return block_size_;
}
unsigned int MemoryPool::inUsedNum()
{
	return capacity_ - top_;
}

NS_MUGGLE_END
