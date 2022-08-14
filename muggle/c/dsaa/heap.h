/******************************************************************************
 *  @file         heap.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec heap
 *
 *  NOTE: this implementation using binary min-heap
 *****************************************************************************/
 
#ifndef MUGGLE_C_DSAA_HEAP_H_
#define MUGGLE_C_DSAA_HEAP_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

/**
 * @brief heap node
 */
typedef struct muggle_heap_node
{
	void *key;    //!< key data of node
	void *value;  //!< value data of node
}muggle_heap_node_t;

/**
 * @brief binary min-heap
 *
 * NOTE: this heap is min-heap, so if cmp return <= 0, node will be parent,
 * parent key less or equal to children's key
 */
typedef struct muggle_heap
{
	uint64_t             capacity; //!< capacity of heap
	uint64_t             size;     //!< number of nodes in heap
	muggle_heap_node_t   *nodes;   //!< nodes array
	muggle_dsaa_data_cmp cmp;      //!< pointer to compare function for data
}muggle_heap_t;

/**
 * @brief initialize heap
 *
 * @param p_heap    pointer to heap
 * @param cmp       pointer to compare function
 * @param capacity  init capacity of heap
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_heap_init(muggle_heap_t *p_heap, muggle_dsaa_data_cmp cmp, size_t capacity);

/**
 * @brief destroy heap
 *
 * @param p_heap            pointer to heap
 * @param key_func_free     function for free key data, if it's NULL, do nothing for key data
 * @param key_pool          the memory pool passed to key_func_free
 * @param value_func_free   function for free value data, if it's NULL, do nothing for value data
 * @param value_pool        the memory pool passed to value_func_free
 */
MUGGLE_C_EXPORT
void muggle_heap_destroy(muggle_heap_t *p_heap,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool);

/**
 * @brief clear heap
 *
 * @param p_heap            pointer to heap
 * @param key_func_free     function for free key data, if it's NULL, do nothing for key data
 * @param key_pool          the memory pool passed to key_func_free
 * @param value_func_free   function for free value data, if it's NULL, do nothing for value data
 * @param value_pool        the memory pool passed to value_func_free
 */
MUGGLE_C_EXPORT
void muggle_heap_clear(muggle_heap_t *p_heap,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool);

/**
 * @brief check whether the heap is empty
 *
 * @param p_heap  pointer to heap
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_heap_is_empty(muggle_heap_t *p_heap);

/**
 * @brief ensure capacity of allocated storage of array list 
 *
 * @param p_heap    pointer to heap
 * @param capacity  capacity of heap
 *
 * @return
 *     true - capacity is big enough or success allocate new capacity
 *     false - capacity is not enough and failed allocate new capacity
 */
MUGGLE_C_EXPORT
bool muggle_heap_ensure_capacity(muggle_heap_t *p_heap, size_t capacity);

/**
 * @brief insert data into heap
 *
 * @param p_heap     pointer to heap
 * @param key        inserted key
 * @param value      inserted value
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_heap_insert(muggle_heap_t *p_heap, void *key, void *value);

/**
 * @brief fetch root from the heap
 *
 * @param p_heap  pointer to heap
 *
 * @return
 *     - on success, return root node
 *     - on failed, return NULL
 */
MUGGLE_C_EXPORT
muggle_heap_node_t* muggle_heap_root(muggle_heap_t *p_heap);

/**
 * @brief delete the root from the heap and return deleted node
 *
 * @param p_heap     pointer to heap
 * @param node       pointer to node that save key and value of deleted root of heap
 *
 * @return if failed extract node from heap, return false, otherwise return true
 */
MUGGLE_C_EXPORT
bool muggle_heap_extract(muggle_heap_t *p_heap, muggle_heap_node_t *node);

/**
 * @brief find node in heap by key
 *
 * @param p_heap     pointer to heap
 * @param data       the data that want to found
 *
 * @return the node that found, if failed, return NULL
 */
MUGGLE_C_EXPORT
muggle_heap_node_t* muggle_heap_find(muggle_heap_t *p_heap, void *data);

// 
/**
 * @brief remove node in heap
 *
 * @param p_heap           pointer to heap
 * @param node             node need to remove
 * @param key_func_free    function for free key data, if it's NULL, do nothing for key data
 * @param key_pool         the memory pool passed to key_func_free
 * @param value_func_free  function for free value data, if it's NULL, do nothing for value data
 * @param value_pool       the memory pool passed to value_func_free
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_heap_remove(muggle_heap_t *p_heap, muggle_heap_node_t *node,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool);

EXTERN_C_END

#endif
