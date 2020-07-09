/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef TRANS_RUNNER_H_
#define TRANS_RUNNER_H_

#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

typedef int (*fn_trans_write)(void *trans_obj, void *data);
typedef void* (*fn_trans_read)(void *trans_obj);

struct write_thread_args
{
	void                     *trans_obj;
	fn_trans_write           fn;
	muggle_benchmark_block_t *blocks;
	int                      num_block;
};

/****************** run thread ******************/
muggle_thread_ret_t write_thread(void *p_arg);

void init_blocks(struct write_thread_args *args, muggle_benchmark_block_t *blocks, int num_thread, int msg_per_write);

void run_thread_trans_benchmark(struct write_thread_args *args, int num_thread, fn_trans_read fn_read);

/****************** report ******************/
void gen_benchmark_report(const char *name, muggle_benchmark_block_t *block, int cnt);

#endif
