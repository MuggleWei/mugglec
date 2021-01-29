/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_DSAA_ARRAY_LIST_H_
#define MUGGLE_C_DSAA_ARRAY_LIST_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

typedef struct muggle_array_list_node
{
	void *data;
}muggle_array_list_node_t;

typedef struct muggle_array_list
{
	muggle_array_list_node_t *nodes;   // nodes array
	uint64_t                 capacity; // capacity of allocated storage
	uint64_t                 size;     // number of elements in array list
}muggle_array_list_t;


// initialize array list
// @param p_array_list  pointer to array list
// @param capacity      init capacity of array list
MUGGLE_CC_EXPORT
bool muggle_array_list_init(muggle_array_list_t *p_array_list, size_t capacity);

// destroy array list
// @param p_array_list   pointer to array list
// @param func_free      function for free data, if it's NULL, do nothing for data
// @param pool           the memory pool passed to func_free
MUGGLE_CC_EXPORT
void muggle_array_list_destroy(muggle_array_list_t *p_array_list, muggle_dsaa_data_free func_free, void *pool);

// detect array list is empty
// @param p_array_list   pointer to array list
MUGGLE_CC_EXPORT
bool muggle_array_list_is_empty(muggle_array_list_t *p_array_list);

// clear array list
// @param p_array_list   pointer to array list
// @param func_free      function for free data, if it's NULL, do nothing for data
// @param pool           the memory pool passed to func_free
MUGGLE_CC_EXPORT
void muggle_array_list_clear(muggle_array_list_t *p_array_list, muggle_dsaa_data_free func_free, void *pool);

// get number of elements in array list
// @param p_array_list   pointer to array list
// @return number of elements in array list
MUGGLE_CC_EXPORT
size_t muggle_array_list_size(muggle_array_list_t *p_array_list);

// ensure capacity of allocated storage of array list 
// @param p_array_list  pointer to array list
// @param capacity      capacity of allocated storage of array list
MUGGLE_CC_EXPORT
bool muggle_array_list_ensure_capacity(muggle_array_list_t *p_array_list, size_t capacity);

// get array list node in specify index
// @param p_array_list  pointer to array list
// @param index         index of array list, valid range: [0, size-1] or [-1, -size]
MUGGLE_CC_EXPORT
muggle_array_list_node_t* muggle_array_list_index(muggle_array_list_t *p_array_list, int index);

// find index of specify data
// @param p_array_list  pointer to array list
// @param index         start index of array list, valid range: [0, size-1] or [-1, -size]
// @param data          the data that want to found
// @param cmp           pointer to compare function
// @return the index of found, if failed, return -1
MUGGLE_CC_EXPORT
int muggle_array_list_find(muggle_array_list_t *p_array_list, int index, void *data, muggle_dsaa_data_cmp cmp);

// insert data before specified index
// @param p_array_list  pointer to array list
// @param index         index of array list, valid range: [0, size-1] or [-1, -size]
// @param data          inserted data
MUGGLE_CC_EXPORT
muggle_array_list_node_t* muggle_array_list_insert(muggle_array_list_t *p_array_list, int index, void *data);

// append data after specified index
// @param p_array_list  pointer to array list
// @param index         index of array list, valid range: [0, size-1] or [-1, -size]
// @param data          appended data
MUGGLE_CC_EXPORT
muggle_array_list_node_t* muggle_array_list_append(muggle_array_list_t *p_array_list, int index, void *data);

// remove data in specified index
// @param p_array_list  pointer to array list
// @param index         index of array list, valid range: [0, size-1] or [-1, -size]
// @param func_free      function for free data, if it's NULL, do nothing for data
// @param pool           the memory pool passed to func_free
MUGGLE_CC_EXPORT
bool muggle_array_list_remove(muggle_array_list_t *p_array_list, int index, muggle_dsaa_data_free func_free, void *pool);

EXTERN_C_END

#endif
