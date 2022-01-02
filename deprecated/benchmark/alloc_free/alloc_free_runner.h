/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef ALLOC_FREE_RUNNER_H_
#define ALLOC_FREE_RUNNER_H_

#include "muggle_benchmark/muggle_benchmark.h"

typedef void* (*fn_alloc)(void *allocator, size_t size);
typedef void (*fn_free)(void *allocator, void *data);

struct alloc_free_args
{
	muggle_benchmark_config_t *cfg;

	int                      cnt_blocks;
	muggle_benchmark_block_t *blocks;
	void                     **data_container;

	muggle_atomic_int        fetch_cursor;
	muggle_atomic_int        alloc_cursor;
	muggle_atomic_int        free_cursor;

	void                     *allocator;
	size_t                   data_size;
	fn_alloc                 cb_alloc;
	fn_free                  cb_free;

	int                      num_alloc_threads;
	int                      num_free_threads;
};

void run_alloc_free_benchmark(const char *name, struct alloc_free_args *args);

void gen_benchmark_report(const char *name, muggle_benchmark_block_t *blocks, muggle_benchmark_config_t *cfg, int cnt);

#endif
