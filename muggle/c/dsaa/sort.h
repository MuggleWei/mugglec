/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_DSAA_SORT_H_
#define MUGGLE_C_DSAA_SORT_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

////////////////////////////////////////////////////////////////
// sort algorithm
// all sort function have same input arguments
// @param ptr    pointer to element pointer array
// @param count  number of elements in the array
// @param cmp    comparison function

typedef bool (*muggle_func_sort)(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

MUGGLE_CC_EXPORT
bool muggle_insertion_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

MUGGLE_CC_EXPORT
bool muggle_shell_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

MUGGLE_CC_EXPORT
bool muggle_heap_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

MUGGLE_CC_EXPORT
bool muggle_merge_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

MUGGLE_CC_EXPORT
bool muggle_quick_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp);

EXTERN_C_END

#endif
