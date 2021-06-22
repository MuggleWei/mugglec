/******************************************************************************
 *  @file         hash_table.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec hash table
 *****************************************************************************/
 
#include "hash_table.h"
#include <string.h>
#include <stdlib.h>
#include "muggle/c/log/log.h"

static uint64_t s_muggle_default_str_hash_func(void *data)
{
	uint64_t hash_val = 0;
	char *p = (char*)data;
	while (*p != '\0')
	{
		hash_val = (hash_val << 5) + *p;
		p++;
	}

	return hash_val;
}

bool muggle_hash_table_init(muggle_hash_table_t *p_hash_table, size_t table_size, hash_func hash, muggle_dsaa_data_cmp cmp, size_t capacity)
{
	if (cmp == NULL)
	{
		return false;
	}

	memset(p_hash_table, 0, sizeof(*p_hash_table));

	if (capacity > 0)
	{
		if (!MUGGLE_DS_CAP_IS_VALID(capacity))
		{
			return false;
		}

		p_hash_table->pool = (muggle_memory_pool_t*)malloc(sizeof(muggle_memory_pool_t));
		if (p_hash_table->pool == NULL)
		{
			return false;
		}

		if (!muggle_memory_pool_init(p_hash_table->pool, (unsigned int)capacity, sizeof(muggle_hash_table_node_t)))
		{
			free(p_hash_table->pool);
			return false;
		}
	}

	if (table_size < 8)
	{
		table_size = HASH_TABLE_SIZE_10007;
	}
	p_hash_table->table_size = table_size;
	p_hash_table->nodes = (muggle_hash_table_node_t*)malloc(sizeof(muggle_hash_table_node_t) * table_size);
	if (p_hash_table->nodes == NULL)
	{
		if (p_hash_table->pool)
		{
			muggle_memory_pool_destroy(p_hash_table->pool);
			free(p_hash_table->pool);
			p_hash_table->pool = NULL;
		}
		return false;
	}
	for (uint64_t i = 0; i < table_size; i++)
	{
		memset(&p_hash_table->nodes[i], 0, sizeof(muggle_hash_table_node_t));
	}

	if (hash == NULL)
	{
		p_hash_table->hash = s_muggle_default_str_hash_func;
	}
	else
	{
		p_hash_table->hash = hash;
	}

	p_hash_table->cmp = cmp;

	return true;
}

void muggle_hash_table_destroy(muggle_hash_table_t *p_hash_table,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool)
{
	// clear hash table
	muggle_hash_table_clear(p_hash_table, key_func_free, key_pool, value_func_free, value_pool);

	// destroy node memroy pool
	if (p_hash_table->pool)
	{
		muggle_memory_pool_destroy(p_hash_table->pool);
		free(p_hash_table->pool);
	}

	// free table
	free(p_hash_table->nodes);
}

void muggle_hash_table_clear(muggle_hash_table_t *p_hash_table,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool)
{
	for (uint64_t i = 0; i < p_hash_table->table_size; i++)
	{
		muggle_hash_table_node_t *head = &p_hash_table->nodes[i];

		muggle_hash_table_node_t *node = head->next;
		muggle_hash_table_node_t *next = NULL;
		while (node)
		{
			next = node->next;

			muggle_hash_table_remove(
				p_hash_table, node, 
				key_func_free, key_pool, 
				value_func_free, value_pool);

			node = next;
		}
	}
}

muggle_hash_table_node_t* muggle_hash_table_find(muggle_hash_table_t *p_hash_table, void *key)
{
	uint64_t hash_val = p_hash_table->hash(key);
	uint64_t idx = hash_val % p_hash_table->table_size;
	muggle_hash_table_node_t *head = &p_hash_table->nodes[idx];
	muggle_hash_table_node_t *node = head->next;
	muggle_hash_table_node_t *next = NULL;
	while (node)
	{
		if (p_hash_table->cmp(node->key, key) == 0)
		{
			return node;
		}
		node = node->next;
	}

	return NULL;
}

muggle_hash_table_node_t* muggle_hash_table_put(muggle_hash_table_t *p_hash_table, void *key, void *value)
{
	uint64_t hash_val = p_hash_table->hash(key);
	uint64_t idx = hash_val % p_hash_table->table_size;
	muggle_hash_table_node_t *head = &p_hash_table->nodes[idx];
	muggle_hash_table_node_t *node = head->next;
	muggle_hash_table_node_t *next = NULL;
	while (node)
	{
		if (p_hash_table->cmp(node->key, key) == 0)
		{
			return NULL;
		}
		node = node->next;
	}

	muggle_hash_table_node_t *new_node = NULL;
	if (p_hash_table->pool)
	{
		new_node = (muggle_hash_table_node_t*)muggle_memory_pool_alloc(p_hash_table->pool);
	}
	else
	{
		new_node = (muggle_hash_table_node_t*)malloc(sizeof(muggle_hash_table_node_t));
	}

	if (new_node == NULL)
	{
		return NULL;
	}

	new_node->key = key;
	new_node->value = value;

	new_node->next = head->next;
	new_node->prev = head;

	head->next = new_node;
	if (new_node->next)
	{
		new_node->next->prev = new_node;
	}

	return new_node;
}

void muggle_hash_table_remove(
	muggle_hash_table_t *p_hash_table, muggle_hash_table_node_t *node,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool)
{
	// erase data
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

	// erase node
	muggle_hash_table_node_t *prev = node->prev;
	muggle_hash_table_node_t *next = node->next;

	MUGGLE_ASSERT(prev != NULL);

	prev->next = next;
	if (next)
	{
		next->prev = prev;
	}

	if (p_hash_table->pool)
	{
		muggle_memory_pool_free(p_hash_table->pool, node);
	}
	else
	{
		free(node);
	}
}
