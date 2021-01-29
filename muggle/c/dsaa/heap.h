/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_DSAA_HEAP_H_
#define MUGGLE_C_DSAA_HEAP_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

typedef struct muggle_heap_node
{
	void *key;    // key data of node
	void *value;  // value data of node
}muggle_heap_node_t;

// binary min-heap
typedef struct muggle_heap
{
	uint64_t             capacity; // capacity of heap
	uint64_t             size;     // number of nodes in heap
	muggle_heap_node_t   *nodes;   // nodes array

	// NOTE: this heap is min-heap, so if cmp return <= 0, node will be parent,
	// parent key less or equal to children's key
	muggle_dsaa_data_cmp cmp;      // pointer to compare function for data
}muggle_heap_t;

// initialize heap
// @param p_heap    pointer to heap
// @param cmp       pointer to compare function
// @param capacity  init capacity of heap
MUGGLE_CC_EXPORT
bool muggle_heap_init(muggle_heap_t *p_heap, muggle_dsaa_data_cmp cmp, size_t capacity);

// destroy heap
// @param p_heap            pointer to heap
// @param key_func_free     function for free key data, if it's NULL, do nothing for key data
// @param key_pool          the memory pool passed to key_func_free
// @param value_func_free   function for free value data, if it's NULL, do nothing for value data
// @param value_pool        the memory pool passed to value_func_free
MUGGLE_CC_EXPORT
void muggle_heap_destroy(muggle_heap_t *p_heap,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool);

// clear heap
// @param p_heap            pointer to heap
// @param key_func_free     function for free key data, if it's NULL, do nothing for key data
// @param key_pool          the memory pool passed to key_func_free
// @param value_func_free   function for free value data, if it's NULL, do nothing for value data
// @param value_pool        the memory pool passed to value_func_free
MUGGLE_CC_EXPORT
void muggle_heap_clear(muggle_heap_t *p_heap,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool);

// check whether the heap is empty
// @param p_heap     pointer to heap
MUGGLE_CC_EXPORT
bool muggle_heap_is_empty(muggle_heap_t *p_heap);

// ensure capacity of allocated storage of array list 
// @param p_heap     pointer to heap
// @param capacity   capacity of heap
MUGGLE_CC_EXPORT
bool muggle_heap_ensure_capacity(muggle_heap_t *p_heap, size_t capacity);

// insert data into heap
// @param p_heap     pointer to heap
// @param key        inserted key
// @param value      inserted value
MUGGLE_CC_EXPORT
bool muggle_heap_insert(muggle_heap_t *p_heap, void *key, void *value);

// delete the root from the heap and return deleted node
// @param p_heap     pointer to heap
// @param node       pointer to node that save key and value of deleted root of heap
// @return if failed extract node from heap, return false, otherwise return true
MUGGLE_CC_EXPORT
bool muggle_heap_extract(muggle_heap_t *p_heap, muggle_heap_node_t *node);

// find node in heap by key
// @param p_heap     pointer to heap
// @param data       the data that want to found
// @return the node that found, if failed, return NULL
MUGGLE_CC_EXPORT
muggle_heap_node_t* muggle_heap_find(muggle_heap_t *p_heap, void *data);

// remove node in heap
// @param p_heap           pointer to heap
// @param node             node need to remove
// @param key_func_free    function for free key data, if it's NULL, do nothing for key data
// @param key_pool         the memory pool passed to key_func_free
// @param value_func_free  function for free value data, if it's NULL, do nothing for value data
// @param value_pool       the memory pool passed to value_func_free
MUGGLE_CC_EXPORT
bool muggle_heap_remove(muggle_heap_t *p_heap, muggle_heap_node_t *node,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool);

EXTERN_C_END

#endif
