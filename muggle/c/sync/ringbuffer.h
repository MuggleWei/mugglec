/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_RINGBUFFER_H_
#define MUGGLE_C_RINGBUFFER_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"
#include "muggle/c/sync/mutex.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_RINGBUFFER_FLAG_SINGLE_WRITER = 0x01, // user guarantee only one writer use this ringbuffer
	MUGGLE_RINGBUFFER_FLAG_SINGLE_READER = 0x02, // user guarantee only one reader use this ringbuffer
	MUGGLE_RINGBUFFER_FLAG_WNUM_GT_CPU = 0x04, // number of writer greater than number of cpu
};

typedef struct muggle_ringbuffer_tag
{
	MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	muggle_atomic_int capacity;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(1);
	int flag;
	int write_mode;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(2);
	muggle_atomic_int next;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(3);
	muggle_atomic_int cursor;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(4);
	muggle_mutex_t write_mutex;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(5);
	void **datas;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(6);
}muggle_ringbuffer_t;

MUGGLE_CC_EXPORT
int muggle_ringbuffer_init(muggle_ringbuffer_t *r, muggle_atomic_int capacity, int flag);

MUGGLE_CC_EXPORT
int muggle_ringbuffer_destroy(muggle_ringbuffer_t *r);

MUGGLE_CC_EXPORT
int muggle_ringbuffer_write(muggle_ringbuffer_t *r, void *data);

MUGGLE_CC_EXPORT
void* muggle_ringbuffer_read(muggle_ringbuffer_t *r, muggle_atomic_int pos);

EXTERN_C_END

#endif
