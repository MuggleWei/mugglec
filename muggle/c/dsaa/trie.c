/******************************************************************************
 *  @file         trie.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec trie
 *****************************************************************************/
 
#include "trie.h"
#include <string.h>
#include <stdlib.h>

static void muggle_trie_erase_node(muggle_trie_t *p_trie, muggle_trie_node_t *node, muggle_dsaa_data_free func_free, void *pool)
{
	for (int i = 0; i < MUGGLE_TRIE_CHILDREN_SIZE; i++)
	{
		if (node->children[i])
		{
			muggle_trie_erase_node(p_trie, node->children[i], func_free, pool);
		}

		if (node->data)
		{
			if (func_free)
			{
				func_free(pool, node->data);
			}
			node->data = NULL;
		}
	}

	if (p_trie->pool)
	{
		muggle_memory_pool_free(p_trie->pool, node);
	}
	else
	{
		free(node);
	}
}

static muggle_trie_node_t* muggle_trie_allocate_node(muggle_trie_t *p_trie)
{
	muggle_trie_node_t *node = NULL;
	if (p_trie->pool)
	{
		node = (muggle_trie_node_t*)muggle_memory_pool_alloc(p_trie->pool);
	}
	else
	{
		node = (muggle_trie_node_t*)malloc(sizeof(muggle_trie_node_t));
	}

	if (node)
	{
		memset(node, 0, sizeof(*node));
	}

	return node;
}

bool muggle_trie_init(muggle_trie_t *p_trie, size_t capacity)
{
	memset(p_trie, 0, sizeof(*p_trie));

	if (capacity > 0)
	{
		if (!MUGGLE_DS_CAP_IS_VALID(capacity))
		{
			return false;
		}

		p_trie->pool = (muggle_memory_pool_t*)malloc(sizeof(muggle_memory_pool_t));
		if (p_trie->pool == NULL)
		{
			return false;
		}

		if (!muggle_memory_pool_init(p_trie->pool, capacity, sizeof(muggle_trie_node_t)))
		{
			free(p_trie->pool);
			return false;
		}
	}

	return true;
}

void muggle_trie_destroy(muggle_trie_t *p_trie, muggle_dsaa_data_free func_free, void *pool)
{
	// clear trie
	for (int i = 0; i < MUGGLE_TRIE_CHILDREN_SIZE; i++)
	{
		if (p_trie->root.children[i])
		{
			muggle_trie_erase_node(p_trie, p_trie->root.children[i], func_free, pool);
		}
	}

	// destroy node memory pool
	if (p_trie->pool)
	{
		muggle_memory_pool_destroy(p_trie->pool);
		free(p_trie->pool);
	}
}

muggle_trie_node_t* muggle_trie_find(muggle_trie_t *p_trie, const char *key)
{
	muggle_trie_node_t *node = &p_trie->root;

	const char *p = key;
	if (*p == '\0')
	{
		return node->children['\0'];
	}

	while (*p != '\0')
	{
		node = node->children[(int)(*p)];
		if (node == NULL)
		{
			break;
		}

		p++;
	}

	return node;
}

muggle_trie_node_t* muggle_trie_insert(muggle_trie_t *p_trie, const char *key, void *value)
{
	muggle_trie_node_t *node = &p_trie->root;
	muggle_trie_node_t *child = NULL;

	const char *p = key;
	if (*p == '\0')
	{
		child = node->children['\0'];
		if (child == NULL)
		{
			child = muggle_trie_allocate_node(p_trie);
			if (child == NULL)
			{
				return NULL;
			}
			else
			{
				child->data = value;
				node->children['\0'] = child;
				return child;
			}
		}
	}

	while (*p != '\0')
	{
		child = node->children[(int)(*p)];
		if (child == NULL)
		{
			child = muggle_trie_allocate_node(p_trie);
			if (child == NULL)
			{
				return NULL;
			}
			node->children[(int)(*p)] = child;
		}
		
		node = child;

		p++;
	}

	if (child)
	{
		child->data = value;
	}

	return child;
}

bool muggle_trie_remove(muggle_trie_t *p_trie, const char *key, muggle_dsaa_data_free func_free, void *pool)
{
	muggle_trie_node_t *node = muggle_trie_find(p_trie, key);
	if (node == NULL)
	{
		return false;
	}

	if (func_free)
	{
		func_free(pool, node->data);
	}
	node->data = NULL;

	return true;
}
