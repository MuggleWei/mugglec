/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef BENCHMARK_TS_MEMORY_POOL_H_
#define BENCHMARK_TS_MEMORY_POOL_H_

#include "alloc_free_runner.h"

void* run_ts_memory_pool_alloc(void *allocator, size_t size);

void run_ts_memory_pool_free(void *allocator, void *data);

#endif
