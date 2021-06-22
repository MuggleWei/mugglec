/******************************************************************************
 *  @file         array_list.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec array list
 *****************************************************************************/
 
#include "array_list.h"
#include <string.h>
#include <stdlib.h>

// get positive index in array list, if failed, return -1
static int muggle_array_list_get_index(muggle_array_list_t *p_array_list, int index)
{
	int idx = 0;
	if (index >= 0)
	{
		if ((uint64_t)index >= p_array_list->size)
		{
			idx = -1;
		}
		else
		{
			idx = index;
		}
	}
	else
	{
		if ((uint64_t)-index > p_array_list->size)
		{
			idx = -1;
		}
		else
		{
			idx = (int)(p_array_list->size - (uint64_t)(-index));
		}
	}

	return idx;
}

bool muggle_array_list_init(muggle_array_list_t *p_array_list, size_t capacity)
{
	memset(p_array_list, 0, sizeof(*p_array_list));

	capacity = capacity == 0 ? 8 : capacity;

	if (!MUGGLE_DS_CAP_IS_VALID(capacity))
	{
		return false;
	}

	p_array_list->nodes = (muggle_array_list_node_t*)malloc(sizeof(muggle_array_list_node_t) * capacity);
	if (p_array_list->nodes == NULL)
	{
		return false;
	}

	p_array_list->capacity = (uint64_t)capacity;

	return true;
}

void muggle_array_list_destroy(muggle_array_list_t *p_array_list, muggle_dsaa_data_free func_free, void *pool)
{
	muggle_array_list_clear(p_array_list, func_free, pool);

	if (p_array_list)
	{
		free(p_array_list->nodes);
		p_array_list = NULL;
	}
}

bool muggle_array_list_is_empty(muggle_array_list_t *p_array_list)
{
	return p_array_list->size == 0 ? true : false;
}

void muggle_array_list_clear(muggle_array_list_t *p_array_list, muggle_dsaa_data_free func_free, void *pool)
{
	if (func_free)
	{
		for (uint64_t i = 0; i < p_array_list->size; i++)
		{
			if (p_array_list->nodes[i].data)
			{
				func_free(pool, p_array_list->nodes[i].data);
			}
		}
	}

	p_array_list->size = 0;
}

size_t muggle_array_list_size(muggle_array_list_t *p_array_list)
{
	return (size_t)p_array_list->size;
}

bool muggle_array_list_ensure_capacity(muggle_array_list_t *p_array_list, size_t capacity)
{
	if (p_array_list->capacity >= (uint64_t)capacity)
	{
		return true;
	}

	if (!MUGGLE_DS_CAP_IS_VALID(capacity))
	{
		return false;
	}

	muggle_array_list_node_t *new_nodes = (muggle_array_list_node_t*)malloc(sizeof(muggle_array_list_node_t) * capacity);
	if (new_nodes == NULL)
	{
		return false;
	}

	for (uint64_t i = 0; i < p_array_list->size; i++)
	{
		new_nodes[i].data = p_array_list->nodes[i].data;
	}

	free(p_array_list->nodes);
	p_array_list->nodes = new_nodes;
	p_array_list->capacity = capacity;

	return true;
}

muggle_array_list_node_t* muggle_array_list_index(muggle_array_list_t *p_array_list, int index)
{
	int idx = muggle_array_list_get_index(p_array_list, index);
	if (idx < 0)
	{
		return NULL;
	}
	else
	{
		return &p_array_list->nodes[idx];
	}
}

int muggle_array_list_find(muggle_array_list_t *p_array_list, int index, void *data, muggle_dsaa_data_cmp cmp)
{
	index = muggle_array_list_get_index(p_array_list, index);
	if (index < 0)
	{
		return -1;
	}

	uint64_t i = (uint64_t)index;
	for (; i < p_array_list->size; i++)
	{
		if (cmp(p_array_list->nodes[i].data, data) == 0)
		{
			return (int)i;
		}
	}

	return -1;
}

muggle_array_list_node_t* muggle_array_list_insert(muggle_array_list_t *p_array_list, int index, void *data)
{
	if (p_array_list->size == p_array_list->capacity)
	{
		if (!muggle_array_list_ensure_capacity(p_array_list, p_array_list->capacity * 2))
		{
			return NULL;
		}
	}

	int idx = muggle_array_list_get_index(p_array_list, index);
	if (idx < 0)
	{
		if ((index == 0 || index == -1) && p_array_list->size == 0)
		{
			idx = 0;
		}
		else
		{
			return NULL;
		}
	}

	for (int i = (int)p_array_list->size - 1; i >= idx; i--)
	{
		p_array_list->nodes[i + 1].data = p_array_list->nodes[i].data;
	}
	p_array_list->nodes[idx].data = data;
	p_array_list->size++;

	return &p_array_list->nodes[idx];
}

muggle_array_list_node_t* muggle_array_list_append(muggle_array_list_t *p_array_list, int index, void *data)
{
	if (p_array_list->size == p_array_list->capacity)
	{
		if (!muggle_array_list_ensure_capacity(p_array_list, p_array_list->capacity * 2))
		{
			return NULL;
		}
	}

	int idx = muggle_array_list_get_index(p_array_list, index);
	if (idx < 0)
	{
		if ((index == 0 || index == -1) && p_array_list->size == 0)
		{
			idx = 0;
		}
		else
		{
			return NULL;
		}
	}

	for (int i = (int)p_array_list->size - 1; i > idx; i--)
	{
		p_array_list->nodes[i + 1].data = p_array_list->nodes[i].data;
	}

	if (p_array_list->size == 0)
	{
		idx = 0;
	}
	else
	{
		idx = idx + 1;
	}
	p_array_list->nodes[idx].data = data;
	p_array_list->size++;

	return &p_array_list->nodes[idx];
}

bool muggle_array_list_remove(muggle_array_list_t *p_array_list, int index, muggle_dsaa_data_free func_free, void *pool)
{
	index = muggle_array_list_get_index(p_array_list, index);
	if (index < 0)
	{
		return 0;
	}

	if (func_free)
	{
		func_free(pool, p_array_list->nodes[index].data);
	}

	for (int i = index; i < (int)p_array_list->size - 1; i++)
	{
		p_array_list->nodes[i].data = p_array_list->nodes[i + 1].data;
	}
	p_array_list->size--;

	return true;
}
