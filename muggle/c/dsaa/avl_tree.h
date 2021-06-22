/******************************************************************************
 *  @file         avl_tree.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec AVL Tree
 *****************************************************************************/
 
#ifndef MUGGLE_C_DSAA_AVL_TREE_H_
#define MUGGLE_C_DSAA_AVL_TREE_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

/**
 * @brief AVL Tree node
 */
typedef struct muggle_avl_tree_node
{
	struct muggle_avl_tree_node *left;       //!< left sub tree node
	struct muggle_avl_tree_node *right;      //!< right sub tree node
	struct muggle_avl_tree_node *parent;     //!< parent node
	int8_t                      balance;     //!< balance factor = height(right_sub_tree) - height(left_sub_tree)
	void                        *key;        //!< key data of node
	void                        *value;      //!< value data of node
}muggle_avl_tree_node_t;

/**
 * @brief AVL Tree
 */
typedef struct muggle_avl_tree
{
	muggle_avl_tree_node_t *root;  //!< root node of avl tree
	muggle_dsaa_data_cmp   cmp;    //!< pointer to compare function for data
	muggle_memory_pool_t   *pool;  //!< memory pool of tree, if it's NULL, use malloc and free by default
}muggle_avl_tree_t;

/**
 * @brief initialize avl tree
 *
 * @param p_avl_tree pointer to avl tree
 * @param cmp        pointer to compare function
 * @param capacity   init capacity for nodes memory pool, if 0, don't use memory pool
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_avl_tree_init(muggle_avl_tree_t *p_avl_tree, muggle_dsaa_data_cmp cmp, size_t capacity);

/**
 * @brief destroy avl tree
 *
 * @param p_avl_tree        pointer to avl tree
 * @param key_func_free     function for free key data, if it's NULL, do nothing for key data
 * @param key_pool          the memory pool passed to key_func_free
 * @param value_func_free   function for free value data, if it's NULL, do nothing for value data
 * @param value_pool        the memory pool passed to value_func_free
 */
MUGGLE_C_EXPORT
void muggle_avl_tree_destroy(muggle_avl_tree_t *p_avl_tree, 
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool);

/**
 * @brief clear avl tree
 *
 * @param p_avl_tree        pointer to avl tree
 * @param key_func_free     function for free key data, if it's NULL, do nothing for key data
 * @param key_pool          the memory pool passed to key_func_free
 * @param value_func_free   function for free value data, if it's NULL, do nothing for value data
 * @param value_pool        the memory pool passed to value_func_free
 */
MUGGLE_C_EXPORT
void muggle_avl_tree_clear(muggle_avl_tree_t *p_avl_tree, 
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool);

/**
 * @brief find node in avl tree
 *
 * @param p_avl_tree  pointer to avl tree
 * @param data        the data that want to found
 *
 * @return the node that found, if failed, return NULL
 */
MUGGLE_C_EXPORT
muggle_avl_tree_node_t* muggle_avl_tree_find(muggle_avl_tree_t *p_avl_tree, void *data);

/**
 * @brief insert data into avl tree
 *
 * @param p_avl_tree  pointer to avl tree
 * @param key         inserted key
 * @param value       inserted value
 *
 * @return return generated node contain inserted data, if NULL, failed insert data
 */
MUGGLE_C_EXPORT
muggle_avl_tree_node_t* muggle_avl_tree_insert(muggle_avl_tree_t *p_avl_tree, void *key, void *value);

/**
 * @brief remove data in avl tree
 *
 * @param p_avl_tree       pointer to avl tree
 * @param node             node need to remove
 * @param key_func_free    function for free key data, if it's NULL, do nothing for key data
 * @param key_pool         the memory pool passed to key_func_free
 * @param value_func_free  function for free value data, if it's NULL, do nothing for value data
 * @param value_pool       the memory pool passed to value_func_free
 */
MUGGLE_C_EXPORT
void muggle_avl_tree_remove(
	muggle_avl_tree_t *p_avl_tree, muggle_avl_tree_node_t *node,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool);

EXTERN_C_END

#endif
