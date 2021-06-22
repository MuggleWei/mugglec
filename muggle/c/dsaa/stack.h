/******************************************************************************
 *  @file         stack.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec stack
 *****************************************************************************/
 
#ifndef MUGGLE_C_DSAA_STACK_H_
#define MUGGLE_C_DSAA_STACK_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

/**
 * @brief stack node
 */
typedef struct muggle_stack_node
{
	void *data;  //!< data pointer
}muggle_stack_node_t;

typedef struct muggle_stack
{
	muggle_stack_node_t *nodes;   //!< nodes array
	uint64_t            capacity; //!< capacity of allocated storage
	uint64_t            top;      //!< stack top position
}muggle_stack_t;

/**
 * @brief initialize stack
 *
 * @param p_stack   pointer to stack
 * @param capacity  init capacity of stack
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_stack_init(muggle_stack_t *p_stack, size_t capacity);

/**
 * @brief destroy stack
 *
 * @param p_stack      pointer to stack
 * @param func_free    function for free data, if it's NULL, do nothing for data
 * @param pool         the memory pool passed to func_free
 */
MUGGLE_C_EXPORT
void muggle_stack_destroy(muggle_stack_t *p_stack, muggle_dsaa_data_free func_free, void *pool);

/**
 * @brief detect stack is empty
 *
 * @param p_stack pointer to stack
 *
 * @return 
 */
MUGGLE_C_EXPORT
bool muggle_stack_is_empty(muggle_stack_t *p_stack);

/**
 * @brief clear stack
 *
 * @param p_stack      pointer to stack
 * @param func_free    function for free data, if it's NULL, do nothing for data
 * @param pool         the memory pool passed to func_free
 */
MUGGLE_C_EXPORT
void muggle_stack_clear(muggle_stack_t *p_stack, muggle_dsaa_data_free func_free, void *pool);

/**
 * @brief get number of elements in stack
 *
 * @param p_array_list   pointer to stack
 *
 * @return number of elements in stack
 */
MUGGLE_C_EXPORT
size_t muggle_stack_size(muggle_stack_t *p_stack);

/**
 * @brief ensure capacity of allocated storage of stack 
 *
 * @param p_array_list  pointer to stack
 * @param capacity      capacity of allocated storage of stack
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_stack_ensure_capacity(muggle_stack_t *p_stack, size_t capacity);

/**
 * @brief push data into stack
 *
 * @param p_array_list  pointer to stack
 * @param data          pushed data
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
muggle_stack_node_t* muggle_stack_push(muggle_stack_t *p_stack, void *data);

/**
 * @brief get top node of stack
 *
 * @param p_array_list  pointer to stack
 *
 * @return top node of stack, if stack is empty, return NULL
 */
MUGGLE_C_EXPORT
muggle_stack_node_t* muggle_stack_top(muggle_stack_t *p_stack);

/**
 * @brief pop top node of stack
 *
 * @param p_array_list  pointer to stack
 * @param func_free    function for free data, if it's NULL, do nothing for data
 * @param pool         the memory pool passed to func_free
 */
MUGGLE_C_EXPORT
void muggle_stack_pop(muggle_stack_t *p_stack, muggle_dsaa_data_free func_free, void *pool);

EXTERN_C_END

#endif
