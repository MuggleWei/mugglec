/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "benchmark_sowr_memory_pool.h"

void* run_sowr_memory_pool_alloc(void *allocator, size_t size)
{
	muggle_sowr_memory_pool_t *pool = (muggle_sowr_memory_pool_t*)allocator;
	return muggle_sowr_memory_pool_alloc(pool);
}

void run_sowr_memory_pool_free(void *allocator, void *data)
{
	muggle_sowr_memory_pool_free(data);
}
