/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef BENCHMARK_ARRAY_BLOCKING_QUEUE_H_
#define BENCHMARK_ARRAY_BLOCKING_QUEUE_H_

#include "trans_runner.h"

int array_blocking_queue_write(void *trans_obj, void *data);
void* array_blocking_queue_read(void *trans_obj);
void run_array_blocking_queue(
	const char *name,
	int flags,
	struct write_thread_args *args,
	int num_thread,
	muggle_benchmark_block_t *blocks);

#endif
