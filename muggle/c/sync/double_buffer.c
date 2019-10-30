/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "double_buffer.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/base/err.h"

int muggle_double_buffer_init(muggle_double_buffer_t *buf, int capacity, int non_blocking)
{
	memset(buf, 0, sizeof(muggle_double_buffer_t));
	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}
	buf->capacity = capacity;
	for (int i = 0; i < 2; ++i)
	{
		muggle_single_buffer_t *p = &buf->buf[i];
		p->cnt = 0;
		p->datas = (void**)malloc(sizeof(void*) * capacity);
		if (p->datas == NULL)
		{
			if (i == 1)
			{
				free(buf->buf[0].datas);
			}
			return MUGGLE_ERR_MEM_ALLOC;
		}
	}
	buf->front = &buf->buf[0];
	buf->back = &buf->buf[1];
	buf->non_blocking = non_blocking;

	int ret = 0;

	ret = muggle_mutex_init(&buf->mutex);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	ret = muggle_condition_variable_init(&buf->cv_not_full);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	ret = muggle_condition_variable_init(&buf->cv_not_empty);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	return MUGGLE_OK;
}

int muggle_double_buffer_destroy(muggle_double_buffer_t *buf)
{
	free(buf->buf[0].datas);
	free(buf->buf[1].datas);
	muggle_mutex_destroy(&buf->mutex);
	muggle_condition_variable_destroy(&buf->cv_not_full);
	muggle_condition_variable_destroy(&buf->cv_not_empty);
	return MUGGLE_OK;
}

int muggle_double_buffer_write(muggle_double_buffer_t *buf, void *data)
{
	int ret = 0;
	ret = muggle_mutex_lock(&buf->mutex);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	muggle_single_buffer_t *p_back = buf->back;
	while (p_back->cnt == buf->capacity)
	{
		if (buf->non_blocking)
		{
			muggle_mutex_unlock(&buf->mutex);
			return MUGGLE_ERR_FULL;
		}
		muggle_condition_variable_wait(&buf->cv_not_full, &buf->mutex, NULL);
		p_back = buf->back;
	}

	p_back->datas[p_back->cnt++] = data;

	muggle_condition_variable_notify_one(&buf->cv_not_empty);
	muggle_mutex_unlock(&buf->mutex);

	return MUGGLE_OK;
}

muggle_single_buffer_t* muggle_double_buffer_read(muggle_double_buffer_t *buf)
{
	int ret = 0;
	ret = muggle_mutex_lock(&buf->mutex);
	if (ret != MUGGLE_OK)
	{
		return NULL;
	}

	while (buf->back->cnt == 0)
	{
		muggle_condition_variable_wait(&buf->cv_not_empty, &buf->mutex, NULL);
	}

	buf->front->cnt = 0;
	muggle_single_buffer_t *tmp = buf->front;
	buf->front = buf->back;
	buf->back = tmp;

	muggle_condition_variable_notify_one(&buf->cv_not_full);
	muggle_mutex_unlock(&buf->mutex);

	return buf->front;
}
