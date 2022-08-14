/******************************************************************************
 *  @file         heap.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec heap
 *****************************************************************************/
 
#include "heap.h"
#include <string.h>
#include <stdlib.h>

bool muggle_heap_init(muggle_heap_t *p_heap, muggle_dsaa_data_cmp cmp, size_t capacity)
{
	if (cmp == NULL)
	{
		return false;
	}

	capacity = capacity == 0 ? 8 : capacity;
	if (!MUGGLE_DS_CAP_IS_VALID(capacity))
	{
		return false;
	}

	memset(p_heap, 0, sizeof(*p_heap));

	// NOTE: cause root node in index 1, so memory capacity = heap->capacity + 1
	p_heap->nodes = (muggle_heap_node_t*)malloc(sizeof(muggle_heap_node_t) * (capacity + 1));
	if (p_heap->nodes == NULL)
	{
		return false;
	}
	p_heap->capacity = capacity;
	p_heap->cmp = cmp;

	return true;
}

void muggle_heap_destroy(muggle_heap_t *p_heap,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool)
{
	// clear heap
	muggle_heap_clear(p_heap, key_func_free, key_pool, value_func_free, value_pool);

	// free nodes
	if (p_heap->nodes)
	{
		free(p_heap->nodes);
		p_heap->nodes = NULL;
	}
}

void muggle_heap_clear(muggle_heap_t *p_heap,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool)
{
	muggle_heap_node_t *node = NULL;
	for (uint64_t i = 1; i <= p_heap->size; i++)
	{
		node = &p_heap->nodes[i];

		if (node->key)
		{
			if (key_func_free)
			{
				key_func_free(key_pool, node->key);
			}
			node->key = NULL;
		}

		if (node->value)
		{
			if (value_func_free)
			{
				value_func_free(value_pool, node->value);
			}
			node->value = NULL;
		}
	}

	p_heap->size = 0;
}

bool muggle_heap_is_empty(muggle_heap_t *p_heap)
{
	return p_heap->size == 0 ? true : false;
}

bool muggle_heap_ensure_capacity(muggle_heap_t *p_heap, size_t capacity)
{
	if (p_heap->capacity >= capacity)
	{
		return true;
	}

	if (!MUGGLE_DS_CAP_IS_VALID(capacity))
	{
		return false;
	}

	// NOTE: cause root node in index 1, so memory capacity = heap->capacity + 1
	muggle_heap_node_t *new_nodes = (muggle_heap_node_t*)malloc(sizeof(muggle_heap_node_t) * (capacity + 1));
	if (new_nodes == NULL)
	{
		return false;
	}

	for (uint64_t i = 0; i < p_heap->size; i++)
	{
		memcpy(&new_nodes[i + 1], &p_heap->nodes[i + 1], sizeof(muggle_heap_node_t));
	}

	free(p_heap->nodes);
	p_heap->nodes = new_nodes;
	p_heap->capacity = capacity;

	return true;
}

bool muggle_heap_insert(muggle_heap_t *p_heap, void *key, void *value)
{
	if (p_heap->capacity == p_heap->size)
	{
		if (!muggle_heap_ensure_capacity(p_heap, p_heap->capacity * 2))
		{
			return false;
		}
	}
	p_heap->size++;

	uint64_t idx = p_heap->size;
	uint64_t parent_idx = 0;
	muggle_heap_node_t *node = NULL;
	muggle_heap_node_t *parent = NULL;
	while (true)
	{
		parent_idx = idx / 2;
		if (parent_idx == 0)
		{
			break;
		}

		node = &p_heap->nodes[idx];
		parent = &p_heap->nodes[parent_idx];

		if (p_heap->cmp(parent->key, key) <= 0)
		{
			break;
		}

		p_heap->nodes[idx].key = parent->key;
		p_heap->nodes[idx].value = parent->value;

		idx /= 2;
	}
	p_heap->nodes[idx].key = key;
	p_heap->nodes[idx].value = value;

	return true;
}

muggle_heap_node_t* muggle_heap_root(muggle_heap_t *p_heap)
{
	if (muggle_heap_is_empty(p_heap))
	{
		return NULL;
	}

	return &p_heap->nodes[1];
}

bool muggle_heap_extract(muggle_heap_t *p_heap, muggle_heap_node_t *node)
{
	if (muggle_heap_is_empty(p_heap))
	{
		return false;
	}

	node->key = p_heap->nodes[1].key;
	node->value = p_heap->nodes[1].value;

	muggle_heap_node_t *last_node = &p_heap->nodes[p_heap->size--];
	uint64_t child = 0;
	uint64_t i = 0;
	for (i = 1; i * 2 <= p_heap->size; i = child)
	{
		// find smaller child
		child = i * 2;
		if (child != p_heap->size &&
			p_heap->cmp(p_heap->nodes[child + 1].key, p_heap->nodes[child].key) < 0)
		{
			child++;
		}

		// percolate one level
		if (p_heap->cmp(last_node->key, p_heap->nodes[child].key) >= 0)
		{
			p_heap->nodes[i].key = p_heap->nodes[child].key;
			p_heap->nodes[i].value = p_heap->nodes[child].value;
		}
		else
		{
			break;
		}
	}
	p_heap->nodes[i].key = last_node->key;
	p_heap->nodes[i].value = last_node->value;

	return true;
}

muggle_heap_node_t* muggle_heap_find(muggle_heap_t *p_heap, void *data)
{
	for (uint64_t i = 1; i <= p_heap->size; i++)
	{
		if (p_heap->cmp(p_heap->nodes[i].key, data) == 0)
		{
			return &p_heap->nodes[i];
		}
	}

	return NULL;
}

bool muggle_heap_remove(muggle_heap_t *p_heap, muggle_heap_node_t *node,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool)
{
	if (muggle_heap_is_empty(p_heap))
	{
		return false;
	}

	int64_t tmp = (int64_t)(node - &p_heap->nodes[0]);
	if (tmp <= 0 || tmp > p_heap->size)
	{
		return false;
	}

	// free node memory
	if (node->key)
	{
		if (key_func_free)
		{
			key_func_free(key_pool, node->key);
		}
		node->key = NULL;
	}

	if (node->value)
	{
		if (value_func_free)
		{
			value_func_free(value_pool, node->value);
		}
		node->value = NULL;
	}

	uint64_t idx = (uint64_t)tmp;
	muggle_heap_node_t *last_node = &p_heap->nodes[p_heap->size--];
	uint64_t parent_idx = 0;
	uint64_t child_idx = 0;
	muggle_heap_node_t *parent = NULL;
	while (true)
	{
		// move up
		parent_idx = idx / 2;
		if (parent_idx != 0)
		{
			parent = &p_heap->nodes[parent_idx];
			if (p_heap->cmp(last_node->key, parent->key) < 0)
			{
				p_heap->nodes[idx].key = parent->key;
				p_heap->nodes[idx].value = parent->value;
				idx = parent_idx;
				continue;
			}
		}

		// percolate
		child_idx = idx * 2;
		if (child_idx <= p_heap->size)
		{
			// find smaller child
			if (child_idx < p_heap->size &&
				p_heap->cmp(p_heap->nodes[child_idx + 1].key, p_heap->nodes[child_idx].key) < 0)
			{
				child_idx++;
			}

			if (p_heap->cmp(last_node->key, p_heap->nodes[child_idx].key) >= 0)
			{
				p_heap->nodes[idx].key = p_heap->nodes[child_idx].key;
				p_heap->nodes[idx].value = p_heap->nodes[child_idx].value;
				idx = child_idx;
				continue;
			}
		}

		break;
	}
	p_heap->nodes[idx].key = last_node->key;
	p_heap->nodes[idx].value = last_node->value;

	return true;
}
