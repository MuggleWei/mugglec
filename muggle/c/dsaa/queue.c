/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "queue.h"
#include <string.h>
#include <stdlib.h>

static void muggle_queue_free_node(muggle_queue_t *p_queue, muggle_queue_node_t *node)
{
	// remove node
	node->prev->next = node->next;
	node->next->prev = node->prev;

	// free node memory
	if (p_queue->pool)
	{
		muggle_memory_pool_free(p_queue->pool, node);
	}
	else
	{
		free(node);
	}

	// change size count
	p_queue->size--;
}

static void muggle_queue_free_data(muggle_queue_t *p_queue, muggle_queue_node_t *node, muggle_dsaa_data_free func_free, void *pool)
{
	if (node->data)
	{
		if (func_free)
		{
			func_free(pool, node->data);
		}
		node->data = NULL;
	}
}

bool muggle_queue_init(muggle_queue_t *p_queue, size_t capacity)
{
	memset(p_queue, 0, sizeof(*p_queue));

	if (capacity > 0)
	{
		if (!MUGGLE_DS_CAP_IS_VALID(capacity))
		{
			return false;
		}

		p_queue->pool = (muggle_memory_pool_t*)malloc(sizeof(muggle_memory_pool_t));
		if (p_queue->pool == NULL)
		{
			return false;
		}

		if (!muggle_memory_pool_init(p_queue->pool, capacity, sizeof(muggle_queue_node_t)))
		{
			free(p_queue->pool);
			return false;
		}
	}

	p_queue->head.next = &p_queue->tail;
	p_queue->tail.prev = &p_queue->head;

	return true;
}

void muggle_queue_destroy(muggle_queue_t *p_queue, muggle_dsaa_data_free func_free, void *pool)
{
	// clear queue
	muggle_queue_clear(p_queue, func_free, pool);

	// destroy node memory pool
	if (p_queue->pool)
	{
		muggle_memory_pool_destroy(p_queue->pool);
		free(p_queue->pool);
	}
}

bool muggle_queue_is_empty(muggle_queue_t *p_queue)
{
	return p_queue->head.next == &p_queue->tail ? true : false;
}

void muggle_queue_clear(muggle_queue_t *p_queue, muggle_dsaa_data_free func_free, void *pool)
{
	muggle_queue_node_t *node = p_queue->head.next;
	muggle_queue_node_t *next_node = NULL;
	while (node != &p_queue->tail)
	{
		next_node = node->next;

		// free datas
		muggle_queue_free_data(p_queue, node, func_free, pool);

		// free nodes
		muggle_queue_free_node(p_queue, node);

		node = next_node;
	}

	p_queue->size = 0;
}

size_t muggle_queue_size(muggle_queue_t *p_queue)
{
	return p_queue->size;
}

muggle_queue_node_t* muggle_queue_enqueue(muggle_queue_t *p_queue, void *data)
{
	muggle_queue_node_t *new_node = NULL;
	if (p_queue->pool)
	{
		new_node = (muggle_queue_node_t*)muggle_memory_pool_alloc(p_queue->pool);
	}
	else
	{
		new_node = (muggle_queue_node_t*)malloc(sizeof(muggle_queue_node_t));
	}
	if (new_node == NULL)
	{
		return NULL;
	}
	new_node->data = data;

	p_queue->size++;

	muggle_queue_node_t *node = p_queue->tail.prev;
	node->next->prev = new_node;
	new_node->next = node->next;
	new_node->prev = node;
	node->next = new_node;

	return new_node;
}

void muggle_queue_dequeue(muggle_queue_t *p_queue, muggle_dsaa_data_free func_free, void *pool)
{
	if (muggle_queue_is_empty(p_queue))
	{
		return;
	}

	muggle_queue_node_t *node = p_queue->head.next;
	
	// free datas
	muggle_queue_free_data(p_queue, node, func_free, pool);

	// free nodes
	muggle_queue_free_node(p_queue, node);
}

muggle_queue_node_t* muggle_queue_front(muggle_queue_t *p_queue)
{
	if (muggle_queue_is_empty(p_queue))
	{
		return NULL;
	}

	return p_queue->head.next;
}
