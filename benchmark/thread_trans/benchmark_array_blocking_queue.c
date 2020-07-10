/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "benchmark_array_blocking_queue.h"

int array_blocking_queue_write(void *trans_obj, void *data)
{
	muggle_array_blocking_queue_t *queue = (muggle_array_blocking_queue_t*)trans_obj;
	return muggle_array_blocking_queue_put(queue, data);
}
void* array_blocking_queue_read(void *trans_obj)
{
	muggle_array_blocking_queue_t *queue = (muggle_array_blocking_queue_t*)trans_obj;
	return muggle_array_blocking_queue_take(queue);
}
void run_array_blocking_queue(
	const char *name,
	int flags,
	struct write_thread_args *args,
	int num_thread,
	muggle_benchmark_block_t *blocks)
{
	MUGGLE_LOG_INFO("run benchmark %s", name);

	init_blocks(args, blocks, num_thread);

	MUGGLE_LOG_INFO("init blocks ok");

	int total_msg_num = num_thread * (int)args->cfg->loop * (int)args->cfg->cnt_per_loop;
	muggle_array_blocking_queue_t queue;
	muggle_atomic_int capacity = total_msg_num / 64;
	if (muggle_array_blocking_queue_init(&queue, capacity))
	{
		MUGGLE_LOG_ERROR("failed init %s with capacity: %d", name, (int)capacity);
		exit(EXIT_FAILURE);
	}

	MUGGLE_LOG_INFO("init %s ok", name);

	for (int i = 0; i < num_thread; i++)
	{
		args[i].fn = array_blocking_queue_write;
		args[i].trans_obj = (void*)&queue;
	}

	MUGGLE_LOG_INFO("start benchmark %s", name);

	run_thread_trans_benchmark(args, num_thread, array_blocking_queue_read);

	MUGGLE_LOG_INFO("benchmark %s completed", name);

	muggle_array_blocking_queue_destroy(&queue);

	MUGGLE_LOG_INFO("gen report for benchmark %s", name);

	gen_benchmark_report(name, blocks, args[0].cfg, total_msg_num);

	MUGGLE_LOG_INFO("report for benchmark %s complete", name);
}
