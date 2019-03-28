/*
*	author: muggle wei <mugglewei@gmail.com>
*
*	Use of this source code is governed by the MIT license that can be
*	found in the LICENSE file.
*/

#ifndef MUGGLE_RINGBUFFER_H_
#define MUGGLE_RINGBUFFER_H_

#include "muggle/c/base/macro.h"

#if MUGGLE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <sched.h>
#endif

#include <stdbool.h>
#include <stdint.h>

NS_MUGGLE_BEGIN

typedef struct MuggleRingBuffer_tag
{
	uint64_t capacity;
	uint64_t block_size;
	volatile int64_t cursor;
	volatile int64_t next;
	void *datas;
} MuggleRingBuffer;

MUGGLE_CC_EXPORT
bool MuggleRingBufferInit(MuggleRingBuffer *ring_buffer, uint64_t capacity, uint64_t block_size);

MUGGLE_CC_EXPORT
void MuggleRingBufferDestory(MuggleRingBuffer *ring_buffer);

MUGGLE_CC_EXPORT
bool MuggleRingBufferWrite(MuggleRingBuffer *ring_buffer, void *data, size_t len);

MUGGLE_CC_EXPORT
int64_t MuggleRingBufferNext(MuggleRingBuffer *ring_buffer);

MUGGLE_CC_EXPORT
void MuggleRingBufferCommit(MuggleRingBuffer *ring_buffer, int64_t idx);

MUGGLE_CC_EXPORT
bool MuggleRingBufferWriteSingleThread(MuggleRingBuffer *ring_buffer, void *data, size_t len);

MUGGLE_CC_EXPORT
int64_t MuggleRingBufferNextSingleThread(MuggleRingBuffer *ring_buffer);

MUGGLE_CC_EXPORT
void MuggleRingBufferCommitSingleThread(MuggleRingBuffer *ring_buffer, int64_t idx);

MUGGLE_CC_EXPORT
void* MuggleRingBufferGet(MuggleRingBuffer *ring_buffer, int64_t idx);

MUGGLE_CC_EXPORT
void* MuggleRingBufferSpinRead(MuggleRingBuffer *ring_buffer, int64_t idx);

MUGGLE_CC_EXPORT
void* MuggleRingBufferYieldRead(MuggleRingBuffer *ring_buffer, int64_t idx);

NS_MUGGLE_END

#endif