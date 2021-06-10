/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_DSAA_TRIE_H_
#define MUGGLE_C_DSAA_TRIE_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

#define MUGGLE_TRIE_CHILDREN_SIZE 256

typedef struct muggle_trie_node
{
	struct muggle_trie_node *children[MUGGLE_TRIE_CHILDREN_SIZE];
	void                    *data;
}muggle_trie_node_t;

typedef struct muggle_trie
{
	muggle_trie_node_t   root;   // root node of tree
	muggle_memory_pool_t *pool;  // memory pool of tree, if it's NULL, use malloc and free by default
}muggle_trie_t;

// initialize trie
// @param p_trie     pointer to trie
// @param capacity   init capacity for nodes memory pool, if 0, don't use memory pool
MUGGLE_C_EXPORT
bool muggle_trie_init(muggle_trie_t *p_trie, size_t capacity);

// destroy trie
// @param p_trie     pointer to trie
// @param func_free  function for free data, if it's NULL, do nothing for data
// @param pool       the memory pool passed to func_free
MUGGLE_C_EXPORT
void muggle_trie_destroy(muggle_trie_t *p_trie, muggle_dsaa_data_free func_free, void *pool);

// find trie node of key
// NOTE: cause trie_remove is lazy remove, node != NULL and node->data == NULL will exists
// @param p_trie     pointer to trie
// @param key        key word
MUGGLE_C_EXPORT
muggle_trie_node_t* muggle_trie_find(muggle_trie_t *p_trie, const char *key);

// insert key value pair
// @param p_trie     pointer to trie
// @param key        key word
// @param value      value insert into trie
// @return return generated node contain added data, if NULL, failed add data
MUGGLE_C_EXPORT
muggle_trie_node_t* muggle_trie_insert(muggle_trie_t *p_trie, const char *key, void *value);

// remove trie node by key
// NOTE: lazy remove, just remove data, it will don't remove node in trie
// @param p_trie     pointer to trie
// @param key        key word
// @param func_free     function for free data, if it's NULL, do nothing for data
// @param pool          the memory pool passed to func_free
MUGGLE_C_EXPORT
bool muggle_trie_remove(muggle_trie_t *p_trie, const char *key, muggle_dsaa_data_free func_free, void *pool);

EXTERN_C_END

#endif
