/******************************************************************************
 *  @file         stack.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec stack
 *****************************************************************************/
 
#include "stack.h"
#include <string.h>
#include <stdlib.h>

bool muggle_stack_init(muggle_stack_t *p_stack, size_t capacity)
{
	memset(p_stack, 0, sizeof(*p_stack));

	capacity = capacity == 0 ? 8 : capacity;

	if (!MUGGLE_DS_CAP_IS_VALID(capacity))
	{
		return false;
	}

	p_stack->nodes = (muggle_stack_node_t*)malloc(sizeof(muggle_stack_node_t) * capacity);
	if (p_stack->nodes == NULL)
	{
		return false;
	}

	p_stack->capacity = capacity;

	return true;
}

void muggle_stack_destroy(muggle_stack_t *p_stack, muggle_dsaa_data_free func_free, void *pool)
{
	muggle_stack_clear(p_stack, func_free, pool);

	free(p_stack->nodes);
}

bool muggle_stack_is_empty(muggle_stack_t *p_stack)
{
	return p_stack->top == 0 ? true : false;
}

void muggle_stack_clear(muggle_stack_t *p_stack, muggle_dsaa_data_free func_free, void *pool)
{
	if (func_free)
	{
		for (uint64_t i = 0; i < p_stack->top; i++)
		{
			if (p_stack->nodes[i].data)
			{
				func_free(pool, p_stack->nodes[i].data);
			}
		}
	}

	p_stack->top = 0;
}

size_t muggle_stack_size(muggle_stack_t *p_stack)
{
	return p_stack->top;
}

bool muggle_stack_ensure_capacity(muggle_stack_t *p_stack, size_t capacity)
{
	if (p_stack->capacity >= capacity)
	{
		return true;
	}

	if (!MUGGLE_DS_CAP_IS_VALID(capacity))
	{
		return false;
	}

	muggle_stack_node_t *new_nodes = (muggle_stack_node_t*)malloc(sizeof(muggle_stack_node_t) * capacity);
	if (new_nodes == NULL)
	{
		return false;
	}

	for (uint64_t i = 0; i < p_stack->top; i++)
	{
		new_nodes[i].data = p_stack->nodes[i].data;
	}

	free(p_stack->nodes);
	p_stack->nodes = new_nodes;
	p_stack->capacity = capacity;

	return true;
}

muggle_stack_node_t* muggle_stack_push(muggle_stack_t *p_stack, void *data)
{
	if (p_stack->top == p_stack->capacity)
	{
		if (!muggle_stack_ensure_capacity(p_stack, p_stack->capacity * 2))
		{
			return NULL;
		}
	}

	muggle_stack_node_t *node = &p_stack->nodes[p_stack->top];
	node->data = data;
	p_stack->top++;

	return node;
}

muggle_stack_node_t* muggle_stack_top(muggle_stack_t *p_stack)
{
	if (p_stack->top == 0)
	{
		return NULL;
	}

	int64_t idx = (int64_t)p_stack->top - 1;
	return &p_stack->nodes[idx];
}

void muggle_stack_pop(muggle_stack_t *p_stack, muggle_dsaa_data_free func_free, void *pool)
{
	if (p_stack->top == 0)
	{
		return;
	}

	p_stack->top--;

	if (func_free != NULL)
	{
		if (p_stack->nodes[p_stack->top].data != NULL)
		{
			func_free(pool, p_stack->nodes[p_stack->top].data);
		}
	}
}
