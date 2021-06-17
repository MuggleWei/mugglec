/******************************************************************************
 *  @file         array_list.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec array list
 *****************************************************************************/

#ifndef MUGGLE_C_DSAA_ARRAY_LIST_H_
#define MUGGLE_C_DSAA_ARRAY_LIST_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

/**
 * @brief mugglec array list node
 */
typedef struct muggle_array_list_node
{
	void *data; //!< data pointer
}muggle_array_list_node_t;

/**
 * @brief mugglec array list
 */
typedef struct muggle_array_list
{
	muggle_array_list_node_t *nodes;   //!< nodes array
	uint64_t                 capacity; //!< capacity of allocated storage
	uint64_t                 size;     //!< number of elements in array list
}muggle_array_list_t;


/**
 * @brief initialize array list
 *
 * @param p_array_list pointer to array list
 * @param capacity     init capacity of array list
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_array_list_init(muggle_array_list_t *p_array_list, size_t capacity);

/**
 * @brief  destroy array list
 *
 * @param p_array_list  pointer to array list
 * @param func_free     function for free data, if it's NULL, do nothing for data
 * @param pool          the memory pool passed to func_free
 */
MUGGLE_C_EXPORT
void muggle_array_list_destroy(muggle_array_list_t *p_array_list, muggle_dsaa_data_free func_free, void *pool);

/**
 * @brief  detect array list is empty
 *
 * @param p_array_list pointer to array list
 *
 * @return 
 */
MUGGLE_C_EXPORT
bool muggle_array_list_is_empty(muggle_array_list_t *p_array_list);

/**
 * @brief  clear array list
 *
 * @param p_array_list pointer to array list
 * @param func_free    function for free data, if it's NULL, do nothing for data
 * @param pool         the memory pool passed to func_free
 */
MUGGLE_C_EXPORT
void muggle_array_list_clear(muggle_array_list_t *p_array_list, muggle_dsaa_data_free func_free, void *pool);

/**
 * @brief  get number of elements in array list
 *
 * @param p_array_list  pointer to array list
 *
 * @return number of elements in array list
 */
MUGGLE_C_EXPORT
size_t muggle_array_list_size(muggle_array_list_t *p_array_list);

/**
 * @brief  ensure capacity of allocated storage of array list 
 *
 * @param p_array_list pointer to array list
 * @param capacity     capacity of allocated storage of array list
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_array_list_ensure_capacity(muggle_array_list_t *p_array_list, size_t capacity);

/**
 * @brief get array list node in specify index
 *
 * @param p_array_list pointer to array list
 * @param index        index of array list, valid range: [0, size-1] or [-1, -size]
 *
 * @return
 *     success - return array list node pointer
 *     failed - return NULL
 */
MUGGLE_C_EXPORT
muggle_array_list_node_t* muggle_array_list_index(muggle_array_list_t *p_array_list, int index);

/**
 * @brief find index of specify data
 *
 * @param p_array_list pointer to array list
 * @param index        start index of array list, valid range: [0, size-1] or [-1, -size]
 * @param data         the data that want to found
 * @param cmp          pointer to compare function
 *
 * @return  the index of found on success, if failed, return -1
 */
MUGGLE_C_EXPORT
int muggle_array_list_find(muggle_array_list_t *p_array_list, int index, void *data, muggle_dsaa_data_cmp cmp);

/**
 * @brief  insert data before specified index
 *
 * @param p_array_list pointer to array list
 * @param index        index of array list, valid range: [0, size-1] or [-1, -size]
 * @param data         inserted data
 *
 * @return 
 *     success - return array list node pointer that inserted
 *     failed - return NULL
 */
MUGGLE_C_EXPORT
muggle_array_list_node_t* muggle_array_list_insert(muggle_array_list_t *p_array_list, int index, void *data);

/**
 * @brief append data after specified index
 *
 * @param p_array_list  pointer to array list
 * @param index         index of array list, valid range: [0, size-1] or [-1, -size]
 * @param data          appended data
 *
 * @return 
 *     success - return array list node pointer that appended
 *     failed - return NULL
 */
MUGGLE_C_EXPORT
muggle_array_list_node_t* muggle_array_list_append(muggle_array_list_t *p_array_list, int index, void *data);

/**
 * @brief  remove data in specified index
 *
 * @param p_array_list pointer to array list
 * @param index        index of array list, valid range: [0, size-1] or [-1, -size]
 * @param func_free    function for free data, if it's NULL, do nothing for data
 * @param pool         the memory pool passed to func_free
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_array_list_remove(muggle_array_list_t *p_array_list, int index, muggle_dsaa_data_free func_free, void *pool);

EXTERN_C_END

#endif
