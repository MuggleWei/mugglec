/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "benchmark_channel.h"

int channel_write(void *trans_obj, void *data)
{
	muggle_channel_t *chan = (muggle_channel_t*)trans_obj;
	return muggle_channel_write(chan, data);
}
void* channel_read(void *trans_obj)
{
	muggle_channel_t *chan = (muggle_channel_t*)trans_obj;
	return muggle_channel_read(chan);
}
void run_channel(
	const char *name,
	int flags,
	struct write_thread_args *args,
	int num_thread,
	muggle_benchmark_block_t *blocks)
{
	MUGGLE_LOG_INFO("run benchmark %s", name);

	init_blocks(args, blocks, num_thread);

	MUGGLE_LOG_INFO("init blocks ok");

	int total_msg_num = (int)(num_thread * args->cfg->loop * args->cfg->cnt_per_loop);
	muggle_channel_t chan;
	muggle_atomic_int capacity = total_msg_num / 64;
	if (muggle_channel_init(&chan, capacity, flags) != 0)
	{
		MUGGLE_LOG_ERROR("failed init %s with capacity: %d", name, (int)capacity);
		exit(EXIT_FAILURE);
	}

	MUGGLE_LOG_INFO("init %s ok", name);

	for (int i = 0; i < num_thread; i++)
	{
		args[i].fn = channel_write;
		args[i].trans_obj = (void*)&chan;
	}

	MUGGLE_LOG_INFO("start benchmark %s", name);

	run_thread_trans_benchmark(args, num_thread, channel_read);

	MUGGLE_LOG_INFO("benchmark %s completed", name);

	muggle_channel_destroy(&chan);

	MUGGLE_LOG_INFO("gen report for benchmark %s", name);

	gen_benchmark_report(name, blocks, args[0].cfg, total_msg_num);

	MUGGLE_LOG_INFO("report for benchmark %s complete", name);
}
