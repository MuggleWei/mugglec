/******************************************************************************
 *  @file         linked_list.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec linked list
 *****************************************************************************/
 
#ifndef MUGGLE_C_DSAA_LINKED_LIST_H_
#define MUGGLE_C_DSAA_LINKED_LIST_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

/**
 * @brief linked list node
 */
typedef struct muggle_linked_list_node
{
	struct muggle_linked_list_node *prev;  //!< previous node
	struct muggle_linked_list_node *next;  //!< next node
	void                           *data;  //!< data pointer
}muggle_linked_list_node_t;

/**
 * @brief linked list
 */
typedef struct muggle_linked_list
{
	muggle_linked_list_node_t head;   //!< head node of linked list
	muggle_linked_list_node_t tail;   //!< tail node of linked list
	muggle_memory_pool_t      *pool;  //!< memory pool of linked list nodes, if it's NULL, use malloc and free by default
	uint64_t                  size;   //!< number of elements in list
}muggle_linked_list_t;

// 
/**
 * @brief initialize linked list
 *
 * @param p_linked_list  pointer to linked list
 * @param capacity       init capacity for nodes memory pool, if 0, don't use memory pool
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_linked_list_init(muggle_linked_list_t *p_linked_list, size_t capacity);

/**
 * @brief destroy linked list
 *
 * @param p_linked_list  pointer to linked list
 * @param func_free      function for free data, if it's NULL, do nothing for data
 * @param pool           the memory pool passed to func_free
 */
MUGGLE_C_EXPORT
void muggle_linked_list_destroy(muggle_linked_list_t *p_linked_list, muggle_dsaa_data_free func_free, void *pool);

/**
 * @brief detect linked list is empty
 *
 * @param p_linked_list  pointer to linked list
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_linked_list_is_empty(muggle_linked_list_t *p_linked_list);

/**
 * @brief clear linked list
 *
 * @param p_linked_list  pointer to linked list
 * @param func_free      function for free data, if it's NULL, do nothing for data
 * @param pool           the memory pool passed to func_free
 */
MUGGLE_C_EXPORT
void muggle_linked_list_clear(muggle_linked_list_t *p_linked_list, muggle_dsaa_data_free func_free, void *pool);

/**
 * @brief get number of elements in linked list
 *
 * @param p_linked_list pointer to linked list
 *
 * @return number of elements in linked list
 */
MUGGLE_C_EXPORT
size_t muggle_linked_list_size(muggle_linked_list_t *p_linked_list);

/**
 * @brief find nodes in linked list
 *
 * @param p_linked_list  pointer to linked list
 * @param node           the node that search start,if is NULL, search start from first node in linked list
 * @param data           the data that want to found
 * @param cmp            pointer to compare function
 *
 * @return the node that found, if failed, return NULL
 */
MUGGLE_C_EXPORT
muggle_linked_list_node_t* muggle_linked_list_find(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node, void *data, muggle_dsaa_data_cmp cmp);

/**
 * @brief get next node of pass in node in linked list
 *
 * @param p_linked_list  pointer to linked list
 * @param node           node in linked list
 *
 * @return the next node of pass in node, if pass in node is last, then return NULL
 */
MUGGLE_C_EXPORT
muggle_linked_list_node_t* muggle_linked_list_next(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node);

/**
 * @brief get previous node of pass in node in linked list
 *
 * @param p_linked_list  pointer to linked list
 * @param node           node in linked list
 *
 * @return the previous node of pass in node, if pass in node is first, then return NULL
 */
MUGGLE_C_EXPORT
muggle_linked_list_node_t* muggle_linked_list_prev(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node);

/**
 * @brief get first node in linked list
 *
 * @param p_linked_list pointer to linked list
 *
 * @return first node in linked list, if linked list is empty, return NULL
 */
MUGGLE_C_EXPORT
muggle_linked_list_node_t* muggle_linked_list_first(muggle_linked_list_t *p_linked_list);

/**
 * @brief get last node in linked list
 *
 * @param p_linked_list pointer to linked list
 *
 * @return first node in linked list, if linked list is empty, return NULL
 */
MUGGLE_C_EXPORT
muggle_linked_list_node_t* muggle_linked_list_last(muggle_linked_list_t *p_linked_list);

/**
 * @brief insert data before specified node
 *
 * @param p_linked_list  pointer to linked list
 * @param node           specified node, if NULL, insert as first node in linked list
 * @param data           inserted data
 * @return return generated node contain inserted data, if NULL, failed insert data
 */
MUGGLE_C_EXPORT
muggle_linked_list_node_t* muggle_linked_list_insert(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node, void *data);

/**
 * @brief append data after specified node
 *
 * @param p_linked_list  pointer to linked list
 * @param node           specified node, if NULL, append as last node in linked list
 * @param data           appended data
 *
 * @return return generated node contain inserted data, if NULL, failed append data
 */
MUGGLE_C_EXPORT
muggle_linked_list_node_t* muggle_linked_list_append(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node, void *data);

/**
 * @brief remove node and return next node
 *
 * @param p_linked_list  pointer to linked list
 * @param node           node need to remove
 * @param func_free      function for free data, if it's NULL, do nothing for data
 * @param pool           the memory pool passed to func_free
 *
 * @return next node of removed node
 */
MUGGLE_C_EXPORT
muggle_linked_list_node_t* muggle_linked_list_remove(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node, muggle_dsaa_data_free func_free, void *pool);

EXTERN_C_END

#endif
