/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_ARRAY_BLOCKING_QUEUE_H_
#define MUGGLE_C_ARRAY_BLOCKING_QUEUE_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/sync/mutex.h"
#include "muggle/c/sync/condition_variable.h"

EXTERN_C_BEGIN

typedef struct muggle_array_blocking_queue_tag
{
	void **datas;
	int capacity;
	int take_idx;
	int put_idx;
	int cnt;
	muggle_mutex_t mutex;
	muggle_condition_variable_t cv_not_empty;
	muggle_condition_variable_t cv_not_full;
}muggle_array_blocking_queue_t;

MUGGLE_C_EXPORT
int muggle_array_blocking_queue_init(muggle_array_blocking_queue_t *queue, int capacity);

MUGGLE_C_EXPORT
int muggle_array_blocking_queue_destroy(muggle_array_blocking_queue_t *queue);

MUGGLE_C_EXPORT
int muggle_array_blocking_queue_put(muggle_array_blocking_queue_t *queue, void *data);

MUGGLE_C_EXPORT
void* muggle_array_blocking_queue_take(muggle_array_blocking_queue_t *queue);

EXTERN_C_END

#endif
