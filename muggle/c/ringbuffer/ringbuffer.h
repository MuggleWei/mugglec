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

EXTERN_C_BEGIN

typedef struct MuggleRingBuffer_tag
{
	int capacity;
	size_t block_size;
	int cursor;
	int next;
	void *datas;
} MuggleRingBuffer;

MUGGLE_CC_EXPORT
bool MuggleRingBufferInit(MuggleRingBuffer *ring_buffer, int capacity, size_t block_size);

MUGGLE_CC_EXPORT
void MuggleRingBufferDestory(MuggleRingBuffer *ring_buffer);

MUGGLE_CC_EXPORT
bool MuggleRingBufferWrite(MuggleRingBuffer *ring_buffer, void *data, size_t len);

MUGGLE_CC_EXPORT
int MuggleRingBufferNext(MuggleRingBuffer *ring_buffer);

MUGGLE_CC_EXPORT
void MuggleRingBufferCommit(MuggleRingBuffer *ring_buffer, int idx);

MUGGLE_CC_EXPORT
bool MuggleRingBufferWriteSingleThread(MuggleRingBuffer *ring_buffer, void *data, size_t len);

MUGGLE_CC_EXPORT
int MuggleRingBufferNextSingleThread(MuggleRingBuffer *ring_buffer);

MUGGLE_CC_EXPORT
void MuggleRingBufferCommitSingleThread(MuggleRingBuffer *ring_buffer, int idx);

MUGGLE_CC_EXPORT
void* MuggleRingBufferGet(MuggleRingBuffer *ring_buffer, int idx);

MUGGLE_CC_EXPORT
void* MuggleRingBufferRead(MuggleRingBuffer *ring_buffer, int idx);

EXTERN_C_END

#endif
