/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_DOUBLE_BUFFER_H_
#define MUGGLE_C_DOUBLE_BUFFER_H_

/*
 * double-buffer just for multiple writer one reader, don't use it
 * with multiple reader
 * */

#include "muggle/c/base/macro.h"
#include "muggle/c/sync/mutex.h"
#include "muggle/c/sync/condition_variable.h"

EXTERN_C_BEGIN

typedef struct muggle_single_buffer_tag
{
	void **datas;
	int cnt;
}muggle_single_buffer_t;

typedef struct muggle_double_buffer_tag
{
	muggle_single_buffer_t buf[2];
	muggle_single_buffer_t *front;
	muggle_single_buffer_t *back;
	int capacity;
	int non_blocking;
	muggle_mutex_t mutex;
	muggle_condition_variable_t cv_not_empty;
	muggle_condition_variable_t cv_not_full;
}muggle_double_buffer_t;

MUGGLE_CC_EXPORT
int muggle_double_buffer_init(muggle_double_buffer_t *buf, int capacity, int non_blocking);

MUGGLE_CC_EXPORT
int muggle_double_buffer_destroy(muggle_double_buffer_t *buf);

MUGGLE_CC_EXPORT
int muggle_double_buffer_write(muggle_double_buffer_t *buf, void *data);

MUGGLE_CC_EXPORT
muggle_single_buffer_t* muggle_double_buffer_read(muggle_double_buffer_t *buf);

EXTERN_C_END

#endif
