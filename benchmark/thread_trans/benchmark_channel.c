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
void run_channel(struct write_thread_args *args, muggle_benchmark_block_t *blocks, int num_thread, int msg_per_write, int read_busy_loop)
{
	MUGGLE_LOG_INFO("run benchmark channel, read_busy_loop=%d", read_busy_loop);

	init_blocks(args, blocks, num_thread, msg_per_write);

	MUGGLE_LOG_INFO("init blocks ok");

	int flags = 0;
	if (read_busy_loop)
	{
		flags = MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP;
	}

	muggle_channel_t chan;
	muggle_atomic_int capacity = num_thread * msg_per_write / 64;
	if (muggle_channel_init(&chan, capacity, flags) != 0)
	{
		MUGGLE_LOG_ERROR("failed init channel with capacity: %d", (int)capacity);
		exit(EXIT_FAILURE);
	}

	MUGGLE_LOG_INFO("init channel ok");

	for (int i = 0; i < num_thread; i++)
	{
		args[i].fn = channel_write;
		args[i].trans_obj = (void*)&chan;
	}

	MUGGLE_LOG_INFO("start benchmark channel");

	run_thread_trans_benchmark(args, num_thread, channel_read);

	MUGGLE_LOG_INFO("benchmark channel completed");

	muggle_channel_destroy(&chan);

	MUGGLE_LOG_INFO("gen report for benchmark channel");

	char name[64];
	if (read_busy_loop)
	{
		snprintf(name, sizeof(name), "channel_%dw_1r", num_thread);
		gen_benchmark_report(name, blocks, num_thread * msg_per_write);
	}
	else
	{
		snprintf(name, sizeof(name), "channel_%dw_1r_busyloop", num_thread);
		gen_benchmark_report(name, blocks, num_thread * msg_per_write);
	}

	MUGGLE_LOG_INFO("report for benchmark channel complete");
}
