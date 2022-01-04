/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef BENCHMARK_RINGBUFFER_H_
#define BENCHMARK_RINGBUFFER_H_

#include "trans_runner.h"

int ringbuffer_write(void *trans_obj, void *data);
void* ringbuffer_read(void *trans_obj);
void run_ringbuffer(
	const char *name,
	int flags,
	struct write_thread_args *args,
	int num_thread,
	muggle_benchmark_block_t *blocks);


#endif
