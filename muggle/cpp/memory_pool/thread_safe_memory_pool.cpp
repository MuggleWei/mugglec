/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/cpp/memory_pool/thread_safe_memory_pool.h"
#include <stdexcpt.h>

NS_MUGGLE_BEGIN

ThreadSafeMemoryPool::ThreadSafeMemoryPool(unsigned int init_capacity, unsigned int block_size)
	: data_bufs_(nullptr)
	, ptr_buf_(nullptr)
	, alloc_index_(0)
	, free_index_(0)
	, used_(0)
	, capacity_(0)
	, block_size_(0)
#if MUGGLE_DEBUG
	, peak_(0)
#endif
{
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

	capacity_ = init_capacity == 0 ? 8 : init_capacity;
	block_size_ = block_size;

	for (unsigned int i = 0; i < init_capacity; ++i)
	{
		ptr_buf_[i] = ((char*)data_bufs_) + i * block_size_;
	}
}
ThreadSafeMemoryPool::~ThreadSafeMemoryPool()
{
	free(data_bufs_);
	free(ptr_buf_);
}

void* ThreadSafeMemoryPool::alloc()
{
	unsigned int used = used_.fetch_add(1);
	if (used >= capacity_)
	{
		used_.fetch_sub(1);
		return nullptr;
	}

#if MUGGLE_DEBUG
	if (used_ > peak_)
	{
		peak_ = used_;
	}
#endif

	unsigned int alloc_idx = alloc_index_.fetch_add(1);
	while (alloc_idx >= capacity_)
	{
		alloc_idx -= capacity_;
	}

	return ptr_buf_[alloc_idx];
}
void ThreadSafeMemoryPool::recycle(void *p)
{
	unsigned int free_idx = free_index_.fetch_add(1);
	while (free_idx >= capacity_)
	{
		free_idx -= capacity_;
	}

	ptr_buf_[free_idx] = p;

	used_.fetch_sub(1);
}

unsigned int ThreadSafeMemoryPool::capacity()
{
	return capacity_;
}
unsigned int ThreadSafeMemoryPool::blockSize()
{
	return block_size_;
}
unsigned int ThreadSafeMemoryPool::inUsedNum()
{
	return used_.load();
}

NS_MUGGLE_END