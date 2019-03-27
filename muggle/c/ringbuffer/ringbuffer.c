/*
*	author: muggle wei <mugglewei@gmail.com>
*
*	Use of this source code is governed by the MIT license that can be
*	found in the LICENSE file.
*/

#include "muggle/c/ringbuffer/ringbuffer.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/base/atomic.h"
#include <stdlib.h>

static inline
uint64_t MuggleRingBufferIndexSlot(int64_t idx, uint64_t capacity)
{
	return (uint64_t)idx & (capacity - 1);
}


bool MuggleRingBufferInit(MuggleRingBuffer *ring_buffer, uint64_t capacity, uint64_t block_size)
{
	ring_buffer->capacity = next_pow_of_2(capacity);
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
	if (len > ring_buffer->block_size)
	{
		return false;
	}

	int64_t idx = MUGGLE_ATOMIC_Add_64(ring_buffer->next, 1);
	uint64_t slot = MuggleRingBufferIndexSlot(idx, ring_buffer->capacity);

	memcpy((void*)((intptr_t)ring_buffer->datas + slot * ring_buffer->block_size), data, len);
	while (MUGGLE_ATOMIC_CAS_64(ring_buffer->cursor, idx, idx + 1) != idx);
	return true;
}

void* MuggleRingBufferGet(MuggleRingBuffer *ring_buffer, int64_t idx)
{
	uint64_t slot = MuggleRingBufferIndexSlot(idx, ring_buffer->capacity);
	return (void*)((intptr_t)ring_buffer->datas + slot * ring_buffer->block_size);
}

void* MuggleRingBufferSpinRead(MuggleRingBuffer *ring_buffer, int64_t idx)
{
	uint64_t slot = MuggleRingBufferIndexSlot(idx, ring_buffer->capacity);
	uint64_t cursor_slot = MuggleRingBufferIndexSlot(ring_buffer->cursor, ring_buffer->capacity);
	while (slot == cursor_slot)
	{
		cursor_slot = MuggleRingBufferIndexSlot(ring_buffer->cursor, ring_buffer->capacity);
	}
	return (void*)((intptr_t)ring_buffer->datas + slot * ring_buffer->block_size);
}

void* MuggleRingBufferYieldRead(MuggleRingBuffer *ring_buffer, int64_t idx)
{
	uint64_t slot = MuggleRingBufferIndexSlot(idx, ring_buffer->capacity);
	uint64_t cursor_slot = MuggleRingBufferIndexSlot(ring_buffer->cursor, ring_buffer->capacity);
	while (idx == ring_buffer->cursor)
	{
#if MUGGLE_PLATFORM_WINDOWS
		SwitchToThread();
#else
		sched_yield();
#endif
	}
	return (void*)((intptr_t)ring_buffer->datas + slot * ring_buffer->block_size);
}