/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "trans_runner.h"
#include "benchmark_channel.h"
#include "benchmark_ringbuffer.h"

int main(int argc, char *argv[])
{
	// init log
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	// convert input arguments
	if (argc < 3)
	{
		MUGGLE_LOG_ERROR("usage: %s <num-thread> <msg-per-write>");
		exit(EXIT_FAILURE);
	}

	int num_thread = 0;
	int msg_per_write = 0;

	if (!muggle_str_toi(argv[1], &num_thread, 10))
	{
		MUGGLE_LOG_ERROR("failed strtoi(num-thread=%s)", argv[1]);
		exit(EXIT_FAILURE);
	}

	if (!muggle_str_toi(argv[2], &msg_per_write, 10))
	{
		MUGGLE_LOG_ERROR("failed strtoi(msg-per-write=%s)", argv[2]);
		exit(EXIT_FAILURE);
	}

	// allocate memory
	int total_msg_num = num_thread * msg_per_write;
	muggle_benchmark_block_t *blocks = (muggle_benchmark_block_t*)malloc(sizeof(muggle_benchmark_block_t) * total_msg_num);
	struct write_thread_args *args = (struct write_thread_args*)malloc(sizeof(struct write_thread_args) * num_thread);

	// benchmark channel
	run_channel(args, blocks, num_thread, msg_per_write, 0);
	run_channel(args, blocks, num_thread, msg_per_write, 1);

	// benchmark ringbuffer
	run_ringbuffer(args, blocks, num_thread, msg_per_write, 0);
	run_ringbuffer(args, blocks, num_thread, msg_per_write, 1);

	// free memory
	free(args);
	free(blocks);

	return 0;
}
