/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "benchmark_ringbuffer.h"

static muggle_atomic_int ringbuf_read_idx = 0;

int ringbuffer_write(void *trans_obj, void *data)
{
	muggle_ring_buffer_t *ringbuf = (muggle_ring_buffer_t*)trans_obj;
	return muggle_ring_buffer_write(ringbuf, data);
}
void* ringbuffer_read(void *trans_obj)
{
	muggle_ring_buffer_t *ringbuf = (muggle_ring_buffer_t*)trans_obj;
	return muggle_ring_buffer_read(ringbuf, ringbuf_read_idx++);
}
void run_ringbuffer(
	const char *name,
	int flags,
	struct write_thread_args *args,
	int num_thread,
	muggle_benchmark_block_t *blocks)
{
	MUGGLE_LOG_INFO("run benchmark %s", name);

	ringbuf_read_idx = 0;

	init_blocks(args, blocks, num_thread);

	MUGGLE_LOG_INFO("init blocks ok");

	int total_msg_num = num_thread * (int)args->cfg->loop * (int)args->cfg->cnt_per_loop;
	muggle_ring_buffer_t ringbuf;
	muggle_atomic_int capacity = total_msg_num / 64;
	if (muggle_ring_buffer_init(&ringbuf, capacity, flags) != 0)
	{
		MUGGLE_LOG_ERROR("failed init ring buffer with capacity: %d", (int)capacity);
		exit(EXIT_FAILURE);
	}

	MUGGLE_LOG_INFO("init ring buffer ok");

	for (int i = 0; i < num_thread; i++)
	{
		args[i].fn = ringbuffer_write;
		args[i].trans_obj = (void*)&ringbuf;
	}

	MUGGLE_LOG_INFO("start benchmark ring buffer");

	run_thread_trans_benchmark(args, num_thread, ringbuffer_read);

	MUGGLE_LOG_INFO("benchmark ring buffer completed");

	muggle_ring_buffer_destroy(&ringbuf);

	MUGGLE_LOG_INFO("gen report for benchmark ring buffer");

	gen_benchmark_report(name, blocks, args[0].cfg, total_msg_num);

	MUGGLE_LOG_INFO("report for benchmark ring buffer complete");
}
