/******************************************************************************
 *  @file         dsaa_utils.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec dsaa utils
 *****************************************************************************/
 
#ifndef MUGGLE_C_DSAA_UTILS_H_
#define MUGGLE_C_DSAA_UTILS_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "muggle/c/memory/memory_pool.h"

EXTERN_C_BEGIN

/**
 * @brief prototype of compare data
 *
 * @param d1  data1
 * @param d2  data2
 * @return
 *     1: d1 > d2
 *     0: d1 == d2
 *     -1: d1 < d2
 */
typedef int(*muggle_dsaa_data_cmp)(const void *d1, const void *d2);

/**
 * @brief prototype of free data
 *
 * @param pool  pointer to a memory pool, it can be NULL
 * @param data  the data wait for free
 */
typedef void(*muggle_dsaa_data_free)(void *pool, void *data);

// check muggle data structure capacity is valid
#define MUGGLE_DS_CAP_IS_VALID(capacity) \
	((uint64_t)(capacity) >= (uint64_t)(1ULL << 31) ? false : true)

EXTERN_C_END

#endif
