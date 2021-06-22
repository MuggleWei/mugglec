/******************************************************************************
 *  @file         trie.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec trie
 *****************************************************************************/
 
#ifndef MUGGLE_C_DSAA_TRIE_H_
#define MUGGLE_C_DSAA_TRIE_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

#define MUGGLE_TRIE_CHILDREN_SIZE 256

/**
 * @brief trie node
 */
typedef struct muggle_trie_node
{
	struct muggle_trie_node *children[MUGGLE_TRIE_CHILDREN_SIZE]; //!< node's children
	void                    *data;                                //!< data pointer
}muggle_trie_node_t;

/**
 * @brief trie
 */
typedef struct muggle_trie
{
	muggle_trie_node_t   root;   //!< root node of trie
	muggle_memory_pool_t *pool;  //!< memory pool of trie, if it's NULL, use malloc and free by default
}muggle_trie_t;

/**
 * @brief initialize trie
 *
 * @param p_trie     pointer to trie
 * @param capacity   init capacity for nodes memory pool, if 0, don't use memory pool
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_trie_init(muggle_trie_t *p_trie, size_t capacity);

/**
 * @brief destroy trie
 *
 * @param p_trie     pointer to trie
 * @param func_free  function for free data, if it's NULL, do nothing for data
 * @param pool       the memory pool passed to func_free
 */
MUGGLE_C_EXPORT
void muggle_trie_destroy(muggle_trie_t *p_trie, muggle_dsaa_data_free func_free, void *pool);

/**
 * @brief find trie node of key
 *
 * NOTE: cause trie_remove is lazy remove, node != NULL and node->data == NULL will exists
 *
 * @param p_trie     pointer to trie
 * @param key        key word
 *
 * @return return pointer of the node on success find key, otherwise return NULL
 */
MUGGLE_C_EXPORT
muggle_trie_node_t* muggle_trie_find(muggle_trie_t *p_trie, const char *key);

/**
 * @brief insert key value pair
 *
 * @param p_trie     pointer to trie
 * @param key        key word
 * @param value      value insert into trie
 *
 * @return return generated node contain added data, if NULL, failed add data
 */
MUGGLE_C_EXPORT
muggle_trie_node_t* muggle_trie_insert(muggle_trie_t *p_trie, const char *key, void *value);

// 
/**
 * @brief remove trie node by key
 *
 * NOTE: lazy remove, just remove data, it will don't remove node in trie
 *
 * @param p_trie     pointer to trie
 * @param key        key word
 * @param func_free     function for free data, if it's NULL, do nothing for data
 * @param pool          the memory pool passed to func_free
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_trie_remove(muggle_trie_t *p_trie, const char *key, muggle_dsaa_data_free func_free, void *pool);

EXTERN_C_END

#endif
