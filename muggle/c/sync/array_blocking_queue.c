/******************************************************************************
 *  @file         array_blocking_queue.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec array blocking queue
 *****************************************************************************/

#include "array_blocking_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "muggle/c/base/err.h"

static void muggle_array_blocking_queue_enqueue(muggle_array_blocking_queue_t *queue, void *data)
{
	queue->datas[queue->put_idx] = data;
	if (++queue->put_idx == queue->capacity)
	{
		queue->put_idx = 0;
	}
	++queue->cnt;
	muggle_condition_variable_notify_one(&queue->cv_not_empty);
}

static void* muggle_array_blocking_queue_dequeue(muggle_array_blocking_queue_t *queue)
{
	void *data = queue->datas[queue->take_idx];
	if (++queue->take_idx == queue->capacity)
	{
		queue->take_idx = 0;
	}
	--queue->cnt;
	muggle_condition_variable_notify_one(&queue->cv_not_full);
	return data;
}

int muggle_array_blocking_queue_init(muggle_array_blocking_queue_t *queue, int capacity)
{
	memset(queue, 0, sizeof(muggle_array_blocking_queue_t));

	if (capacity <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}
	queue->capacity = capacity;
	queue->datas = (void**)malloc(sizeof(void*) * capacity);
	if (queue->datas == NULL)
	{
		return MUGGLE_ERR_MEM_ALLOC;
	}
	queue->take_idx = 0;
	queue->put_idx = 0;
	queue->cnt = 0;

	int ret = 0;

	ret = muggle_mutex_init(&queue->mutex);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	ret = muggle_condition_variable_init(&queue->cv_not_full);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	ret = muggle_condition_variable_init(&queue->cv_not_empty);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	return MUGGLE_OK;
}

int muggle_array_blocking_queue_destroy(muggle_array_blocking_queue_t *queue)
{
	free(queue->datas);
	muggle_mutex_destroy(&queue->mutex);
	muggle_condition_variable_destroy(&queue->cv_not_full);
	muggle_condition_variable_destroy(&queue->cv_not_empty);
	return MUGGLE_OK;
}

int muggle_array_blocking_queue_put(muggle_array_blocking_queue_t *queue, void *data)
{
	int ret = 0;
	ret = muggle_mutex_lock(&queue->mutex);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	while (queue->cnt == queue->capacity)
	{
		muggle_condition_variable_wait(&queue->cv_not_full, &queue->mutex, NULL);
	}
	muggle_array_blocking_queue_enqueue(queue, data);

	muggle_mutex_unlock(&queue->mutex);

	return MUGGLE_OK;
}

void* muggle_array_blocking_queue_take(muggle_array_blocking_queue_t *queue)
{
	int ret = 0;
	ret = muggle_mutex_lock(&queue->mutex);
	if (ret != MUGGLE_OK)
	{
		return NULL;
	}

	while (queue->cnt == 0)
	{
		muggle_condition_variable_wait(&queue->cv_not_empty, &queue->mutex, NULL);
	}
	void *data = muggle_array_blocking_queue_dequeue(queue);

	muggle_mutex_unlock(&queue->mutex);

	return data;
}
