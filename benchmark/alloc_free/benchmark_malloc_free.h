/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef BENCHMARK_MALLOC_FREE_H_
#define BENCHMARK_MALLOC_FREE_H_

#include "alloc_free_runner.h"

void* run_malloc(void *allocator, size_t size);

void run_free(void *allocator, void *data);

#endif
