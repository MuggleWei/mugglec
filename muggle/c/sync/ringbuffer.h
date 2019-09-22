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

EXTERN_C_BEGIN

typedef struct muggle_ringbuffer_tag
{
	MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	muggle_atomic_int capacity;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(1);
	muggle_atomic_int next;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(2);
	muggle_atomic_int cursor;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(3);
	void **datas;
	MUGGLE_STRUCT_CACHE_LINE_PADDING(4);
}muggle_ringbuffer_t;

MUGGLE_CC_EXPORT
int muggle_ringbuffer_init(muggle_ringbuffer_t *r, muggle_atomic_int capacity);

MUGGLE_CC_EXPORT
int muggle_ringbuffer_destroy(muggle_ringbuffer_t *r);

/*
 * NOTE:
 * @ only_one_consumer: when have more than one consumer wait for wakeup, need set to 0, otherwise 1
 * @ producer_need_yield: when number of producer greater than number of CPUs, need set to 1, if set 0 at
 *   that situation, busy loop may seize CPU resources meaningless
 * */
MUGGLE_CC_EXPORT
int muggle_ringbuffer_push(muggle_ringbuffer_t *r, void *data, int only_one_consumer, int producer_need_yield);

/*
 * single thread push
 * user need ensure only one thread allowed push data into this ringbuffer, 
 * and if you used _st_push in a ringbuffer, don't use _push in it again
 * */
MUGGLE_CC_EXPORT
int muggle_ringbuffer_st_push(muggle_ringbuffer_t *r, void *data, int only_one_consumer);

MUGGLE_CC_EXPORT
void* muggle_ringbuffer_get(muggle_ringbuffer_t *r, muggle_atomic_int pos);

MUGGLE_CC_EXPORT
void* muggle_ringbuffer_get_with_cache(
	muggle_ringbuffer_t *r,
	muggle_atomic_int pos,
	muggle_atomic_int *cursor_cache
);

EXTERN_C_END

#endif
