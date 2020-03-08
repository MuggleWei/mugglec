/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "thread_safe_memory_pool.h"
#include <malloc.h>
#include <exception>
#include <new>

NS_MUGGLE_BEGIN

ThreadSafeMemoryPool::ThreadSafeMemoryPool(unsigned int init_capacity, unsigned int block_size)
	: head_(nullptr)
	, data_buf_(nullptr)
	, used_(0)
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

	unsigned int real_block_size = block_size_ + sizeof(void*);

	data_buf_ = (void*)malloc(real_block_size * capacity_);
	if (data_buf_ == nullptr)
	{
		throw std::bad_alloc();
	}

	for (unsigned int i = 0; i < capacity_; ++i)
	{
		char *addr = (char*)(data_buf_) + i * real_block_size;
		Node *node = (Node*)addr;
		node->next = addr + real_block_size;
	}
	Node* last_node = (Node*)((char*)(data_buf_) + (capacity_ - 1) * real_block_size);
	last_node->next = nullptr;

	Node* first_node = (Node*)data_buf_;
	head_.store(first_node);
}
ThreadSafeMemoryPool::~ThreadSafeMemoryPool()
{
	if (data_buf_)
	{
		free(data_buf_);
	}
}

void* ThreadSafeMemoryPool::alloc()
{
	Node *next_pos;
	Node *pos = head_.load();
	do
	{
		if (pos == nullptr)
		{
			return nullptr;
		}
		next_pos = (Node*)pos->next;
	} while (!head_.compare_exchange_weak(pos, next_pos));

	used_++;

	return pos->data();
}
void ThreadSafeMemoryPool::recycle(void *p)
{
	Node *pos = (Node*)((char*)p - sizeof(void*));
	Node *head_pos = head_.load();
	do
	{
		pos->next = head_pos;
	} while (!head_.compare_exchange_weak(head_pos, pos));

	used_--;
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
