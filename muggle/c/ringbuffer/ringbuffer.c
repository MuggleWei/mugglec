/*
*	author: muggle wei <mugglewei@gmail.com>
*
*	Use of this source code is governed by the MIT license that can be
*	found in the LICENSE file.
*/

#include "muggle/c/base/log.h"
#include "muggle/c/ringbuffer/ringbuffer.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/base/atomic.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if MUGGLE_PLATFORM_LINUX 
#include <limits.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <linux/unistd.h>

static int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
	return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
}

#endif

static inline
int MuggleRingBufferIndexSlot(int idx, int capacity)
{
	return idx & (capacity - 1);
}


bool MuggleRingBufferInit(MuggleRingBuffer *ring_buffer, int capacity, size_t block_size)
{
	ring_buffer->capacity = (int)next_pow_of_2((uint64_t)capacity);
	if (ring_buffer->capacity <= 1)
	{
		return false;
	}
	ring_buffer->block_size = block_size;
	ring_buffer->next = 0;
	ring_buffer->cursor = 0;
	ring_buffer->datas = malloc(ring_buffer->capacity * ring_buffer->block_size);
	if (ring_buffer->datas == NULL)
	{
		return false;
	}
	return true;
}

void MuggleRingBufferDestory(MuggleRingBuffer *ring_buffer)
{
	free(ring_buffer->datas);
}

bool MuggleRingBufferWrite(MuggleRingBuffer *ring_buffer, void *data, size_t len)
{
	MUGGLE_ASSERT(len <= ring_buffer->block_size);

	int idx = MUGGLE_ATOMIC_Fetch_And_Add(ring_buffer->next, 1);
	int slot = MuggleRingBufferIndexSlot(idx, ring_buffer->capacity);
	int next_slot = MuggleRingBufferIndexSlot(slot+1, ring_buffer->capacity); 

	memcpy((void*)((intptr_t)ring_buffer->datas + slot * ring_buffer->block_size), data, len);
	while (MUGGLE_ATOMIC_CAS(ring_buffer->cursor, slot, next_slot) != slot);

#if MUGGLE_PLATFORM_LINUX
	futex(&ring_buffer->cursor, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, NULL, 0);
#elif MUGGLE_PLATFORM_WINDOWS
	WakeByAddressAll(&ring_buffer->cursor);
#endif

	return true;
}

int MuggleRingBufferNext(MuggleRingBuffer *ring_buffer)
{
	return MUGGLE_ATOMIC_Fetch_And_Add(ring_buffer->next, 1);
}

void MuggleRingBufferCommit(MuggleRingBuffer *ring_buffer, int idx)
{
	int slot = MuggleRingBufferIndexSlot(idx, ring_buffer->capacity);
	int next_slot = MuggleRingBufferIndexSlot(slot+1, ring_buffer->capacity); 

	while (MUGGLE_ATOMIC_CAS(ring_buffer->cursor, slot, next_slot) != slot);
#if MUGGLE_PLATFORM_LINUX
	futex(&ring_buffer->cursor, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, NULL, 0);
#elif MUGGLE_PLATFORM_WINDOWS
	WakeByAddressAll(&ring_buffer->cursor);
#endif
}

bool MuggleRingBufferWriteSingleThread(MuggleRingBuffer *ring_buffer, void *data, size_t len)
{
	MUGGLE_ASSERT(len <= ring_buffer->block_size);

	int idx = ring_buffer->next++;
	int slot = MuggleRingBufferIndexSlot(idx, ring_buffer->capacity);
	int next_slot = MuggleRingBufferIndexSlot(slot+1, ring_buffer->capacity); 

	memcpy((void*)((intptr_t)ring_buffer->datas + slot * ring_buffer->block_size), data, len);
	ring_buffer->cursor = next_slot;

#if MUGGLE_PLATFORM_LINUX
	futex(&ring_buffer->cursor, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, NULL, 0);
#elif MUGGLE_PLATFORM_WINDOWS
	WakeByAddressAll(&ring_buffer->cursor);
#endif

	return true;
}

int MuggleRingBufferNextSingleThread(MuggleRingBuffer *ring_buffer)
{
	return ring_buffer->next++;
}

void MuggleRingBufferCommitSingleThread(MuggleRingBuffer *ring_buffer, int idx)
{
	int next_slot = MuggleRingBufferIndexSlot(idx+1, ring_buffer->capacity); 
	ring_buffer->cursor = next_slot;

#if MUGGLE_PLATFORM_LINUX
	futex(&ring_buffer->cursor, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, NULL, 0);
#elif MUGGLE_PLATFORM_WINDOWS
	WakeByAddressAll(&ring_buffer->cursor);
#endif
}

void* MuggleRingBufferGet(MuggleRingBuffer *ring_buffer, int idx)
{
	int slot = MuggleRingBufferIndexSlot(idx, ring_buffer->capacity);
	return (void*)((intptr_t)ring_buffer->datas + slot * ring_buffer->block_size);
}

void* MuggleRingBufferRead(MuggleRingBuffer *ring_buffer, int idx)
{
	int slot = MuggleRingBufferIndexSlot(idx, ring_buffer->capacity);
	while (slot == ring_buffer->cursor)
	{
#if MUGGLE_PLATFORM_LINUX
		futex(&ring_buffer->cursor, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, slot, NULL, NULL, 0);
#elif MUGGLE_PLATFORM_WINDOWS
		WaitOnAddress(&ring_buffer->cursor, &slot, sizeof(ring_buffer->cursor), INFINITE);
#else
		sched_yield();
#endif
	}

	return (void*)((intptr_t)ring_buffer->datas + slot * ring_buffer->block_size);
}

