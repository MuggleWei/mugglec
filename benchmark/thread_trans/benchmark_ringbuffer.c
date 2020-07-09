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
void run_ringbuffer(struct write_thread_args *args, muggle_benchmark_block_t *blocks, int num_thread, int msg_per_write, int read_busy_loop)
{
	MUGGLE_LOG_INFO("run benchmark ring buffer, read_busy_loop=%d", read_busy_loop);

	ringbuf_read_idx = 0;

	init_blocks(args, blocks, num_thread, msg_per_write);

	MUGGLE_LOG_INFO("init blocks ok");

	int flags = MUGGLE_RING_BUFFER_FLAG_SINGLE_READER | MUGGLE_RING_BUFFER_FLAG_WRITE_BUSY_LOOP;
	if (read_busy_loop)
	{
		flags |= MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP;
	}

	muggle_ring_buffer_t ringbuf;
	muggle_atomic_int capacity = num_thread * msg_per_write / 64;
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

	char name[64];
	if (read_busy_loop)
	{
		snprintf(name, sizeof(name), "ringbuffer_%dw_1r", num_thread);
		gen_benchmark_report(name, blocks, num_thread * msg_per_write);
	}
	else
	{
		snprintf(name, sizeof(name), "ringbuffer_%dw_1r_busyloop", num_thread);
		gen_benchmark_report(name, blocks, num_thread * msg_per_write);
	}

	MUGGLE_LOG_INFO("report for benchmark ring buffer complete");
}
