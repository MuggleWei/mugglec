/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "linked_list.h"
#include <string.h>
#include <stdlib.h>

// linked list allocate node
static muggle_linked_list_node_t* muggle_linked_list_allocate_node(muggle_linked_list_t *p_linked_list)
{
	muggle_linked_list_node_t *node = NULL;
	if (p_linked_list->pool)
	{
		node = (muggle_linked_list_node_t*)muggle_memory_pool_alloc(p_linked_list->pool);
	}
	else
	{
		node = (muggle_linked_list_node_t*)malloc(sizeof(muggle_linked_list_node_t));
	}

	// change size count
	if (node)
	{
		p_linked_list->size++;
	}
	
	return node;
}

// linked list free node
static void muggle_linked_list_free_node(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node)
{
	// remove node
	node->prev->next = node->next;
	node->next->prev = node->prev;

	// free node memory
	if (p_linked_list->pool)
	{
		muggle_memory_pool_free(p_linked_list->pool, node);
	}
	else
	{
		free(node);
	}

	// change size count
	p_linked_list->size--;
}

// linked list free data
static void muggle_linked_list_free_data(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node, muggle_dsaa_data_free func_free, void *pool)
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

bool muggle_linked_list_init(muggle_linked_list_t *p_linked_list, size_t capacity)
{
	memset(p_linked_list, 0, sizeof(*p_linked_list));

	if (capacity > 0)
	{
		if (!MUGGLE_DS_CAP_IS_VALID(capacity))
		{
			return false;
		}

		p_linked_list->pool = (muggle_memory_pool_t*)malloc(sizeof(muggle_memory_pool_t));
		if (p_linked_list->pool == NULL)
		{
			return false;
		}

		if (!muggle_memory_pool_init(p_linked_list->pool, capacity, sizeof(muggle_linked_list_node_t)))
		{
			free(p_linked_list->pool);
			return false;
		}
	}

	p_linked_list->head.next = &p_linked_list->tail;
	p_linked_list->tail.prev = &p_linked_list->head;

	return true;
}

void muggle_linked_list_destroy(muggle_linked_list_t *p_linked_list, muggle_dsaa_data_free func_free, void *pool)
{
	// clear linked list
	muggle_linked_list_clear(p_linked_list, func_free, pool);

	// destroy node memory pool
	if (p_linked_list->pool)
	{
		muggle_memory_pool_destroy(p_linked_list->pool);
		free(p_linked_list->pool);
	}
}

bool muggle_linked_list_is_empty(muggle_linked_list_t *p_linked_list)
{
	return p_linked_list->head.next == &p_linked_list->tail ? true : false;
}

void muggle_linked_list_clear(muggle_linked_list_t *p_linked_list, muggle_dsaa_data_free func_free, void *pool)
{
	muggle_linked_list_node_t *node = p_linked_list->head.next;
	muggle_linked_list_node_t *next_node = NULL;
	while (node != &p_linked_list->tail)
	{
		next_node = node->next;

		// free datas
		muggle_linked_list_free_data(p_linked_list, node, func_free, pool);

		// free nodes
		muggle_linked_list_free_node(p_linked_list, node);

		node = next_node;
	}

	p_linked_list->size = 0;
}

size_t muggle_linked_list_size(muggle_linked_list_t *p_linked_list)
{
	return p_linked_list->size;
}

muggle_linked_list_node_t* muggle_linked_list_find(muggle_linked_list_t *p_linked_list, struct muggle_linked_list_node *node, void *data, muggle_dsaa_data_cmp cmp)
{
	if (node == NULL)
	{
		node = p_linked_list->head.next;
	}

	while (node != &p_linked_list->tail)
	{
		if (cmp(node->data, data) == 0)
		{
			return node;
		}

		node = node->next;
	}

	return NULL;
}

muggle_linked_list_node_t* muggle_linked_list_next(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node)
{
	muggle_linked_list_node_t *next_node = node->next;
	if (next_node == &p_linked_list->tail)
	{
		return NULL;
	}

	return next_node;
}

muggle_linked_list_node_t* muggle_linked_list_prev(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node)
{
	muggle_linked_list_node_t *prev_node = node->prev;
	if (prev_node == &p_linked_list->head)
	{
		return NULL;
	}

	return prev_node;
}

muggle_linked_list_node_t* muggle_linked_list_first(muggle_linked_list_t *p_linked_list)
{
	muggle_linked_list_node_t *node = p_linked_list->head.next;
	if (node == &p_linked_list->tail)
	{
		return NULL;
	}

	return node;
}

muggle_linked_list_node_t* muggle_linked_list_last(muggle_linked_list_t *p_linked_list)
{
	muggle_linked_list_node_t *node = p_linked_list->tail.prev;
	if (node == &p_linked_list->head)
	{
		return NULL;
	}

	return node;
}

muggle_linked_list_node_t* muggle_linked_list_insert(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node, void *data)
{
	muggle_linked_list_node_t *new_node = muggle_linked_list_allocate_node(p_linked_list);
	if (new_node == NULL)
	{
		return NULL;
	}
	new_node->data = data;

	if (node == NULL)
	{
		node = p_linked_list->head.next;
	}

	node->prev->next = new_node;
	new_node->prev = node->prev;
	new_node->next = node;
	node->prev = new_node;

	return new_node;
}

muggle_linked_list_node_t* muggle_linked_list_append(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node, void *data)
{
	muggle_linked_list_node_t *new_node = muggle_linked_list_allocate_node(p_linked_list);
	if (new_node == NULL)
	{
		return NULL;
	}
	new_node->data = data;

	if (node == NULL)
	{
		node = p_linked_list->tail.prev;
	}

	node->next->prev = new_node;
	new_node->next = node->next;
	new_node->prev = node;
	node->next = new_node;

	return new_node;
}

muggle_linked_list_node_t* muggle_linked_list_remove(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node, muggle_dsaa_data_free func_free, void *pool)
{
	muggle_linked_list_node_t *next_node = muggle_linked_list_next(p_linked_list, node);

	// free datas
	muggle_linked_list_free_data(p_linked_list, node, func_free, pool);

	// free nodes
	muggle_linked_list_free_node(p_linked_list, node);

	return next_node;
}
