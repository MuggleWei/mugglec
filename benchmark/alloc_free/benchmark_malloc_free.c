/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "benchmark_malloc_free.h"

void* run_malloc(void *allocator, size_t size)
{
	return malloc(size);
}

void run_free(void *allocator, void *data)
{
	free(data);
}
