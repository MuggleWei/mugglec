/******************************************************************************
 *  @file         sort.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sort
 *****************************************************************************/
 
#ifndef MUGGLE_C_DSAA_SORT_H_
#define MUGGLE_C_DSAA_SORT_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

/**
 * @brief prototype of sort algorithm function
 *
 * all sort function have same input arguments
 *
 * @param ptr    pointer to element pointer array
 * @param count  number of elements in the array
 * @param cmp    comparison function
 *
 * @return boolean
 */
typedef bool (*muggle_func_sort)(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

MUGGLE_C_EXPORT
bool muggle_insertion_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

MUGGLE_C_EXPORT
bool muggle_shell_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

MUGGLE_C_EXPORT
bool muggle_heap_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

MUGGLE_C_EXPORT
bool muggle_merge_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

MUGGLE_C_EXPORT
bool muggle_quick_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

EXTERN_C_END

#endif
