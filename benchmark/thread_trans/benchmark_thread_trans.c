/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "trans_runner.h"
#include "benchmark_channel.h"
#include "benchmark_ringbuffer.h"

#define PARAM_NUM 5

int get_argv(int argc, int idx, char **argv, const char *name, int default_val)
{
	int val;
	if (idx + 1 >= PARAM_NUM || !muggle_str_toi(argv[idx], &val, 10))
	{
		MUGGLE_LOG_WARNING("failed get value of %s, use default val: %d", name, default_val);
		val = default_val;
	}

	return val;
}

int main(int argc, char *argv[])
{
	// init log
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	// convert input arguments
	if (argc < PARAM_NUM)
	{
		MUGGLE_LOG_WARNING("usage: %s <num-thread> <rounds> <round-interval-ms> <msg-per-round>", argv[0]);
		MUGGLE_LOG_WARNING("missing arguments will use default value");
	}

	int num_thread = get_argv(argc, 1, argv, "num-thread", 4);
	int rounds = get_argv(argc, 2, argv, "rounds", 10);
	int round_interval = get_argv(argc, 3, argv, "round-interval-ms", 1);
	int msg_per_round = get_argv(argc, 4, argv, "msg-per-round", 10000);

	MUGGLE_LOG_INFO("num_thread: %d", num_thread);
	MUGGLE_LOG_INFO("rounds: %d", rounds);
	MUGGLE_LOG_INFO("round_interval: %d", round_interval);
	MUGGLE_LOG_INFO("msg_per_round: %d", msg_per_round);

	muggle_benchmark_config_t benchmark_cfg;
	memset(&benchmark_cfg, 0, sizeof(benchmark_cfg));
	benchmark_cfg.loop = rounds;
	benchmark_cfg.loop_interval_ms = round_interval;
	benchmark_cfg.cnt_per_loop = msg_per_round;
	benchmark_cfg.report_step = 10;

	// allocate memory
	int total_msg_num = num_thread * rounds * msg_per_round;
	muggle_benchmark_block_t *blocks = (muggle_benchmark_block_t*)malloc(sizeof(muggle_benchmark_block_t) * total_msg_num);
	struct write_thread_args *args = (struct write_thread_args*)malloc(sizeof(struct write_thread_args) * num_thread);
	for (int i = 0; i < num_thread; i++)
	{
		args[i].cfg = &benchmark_cfg;
	}

	int flags = 0;
	char name[64];

	// benchmark channel
	MUGGLE_LOG_INFO("=======================================================");
	flags = 0;
	snprintf(name, sizeof(name), "channel_%dw_1r", num_thread);
	run_channel(name, flags, args, num_thread, blocks);

	MUGGLE_LOG_INFO("=======================================================");
	flags = MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP;
	snprintf(name, sizeof(name), "channel_%dw_1r_busyloop", num_thread);
	run_channel(name, flags, args, num_thread, blocks);

	// benchmark ringbuffer
	MUGGLE_LOG_INFO("=======================================================");
	flags = MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK | MUGGLE_RING_BUFFER_FLAG_SINGLE_READER;
	snprintf(name, sizeof(name), "ringbuffer_%dw_lock_1r_single", num_thread);
	run_ringbuffer(name, flags, args, num_thread, blocks);

	MUGGLE_LOG_INFO("=======================================================");
	flags = MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK | MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP;
	snprintf(name, sizeof(name), "ringbuffer_%dw_lock_1r_busyloop", num_thread);
	run_ringbuffer(name, flags, args, num_thread, blocks);

	MUGGLE_LOG_INFO("=======================================================");
	flags = MUGGLE_RING_BUFFER_FLAG_WRITE_BUSY_LOOP | MUGGLE_RING_BUFFER_FLAG_SINGLE_READER;
	snprintf(name, sizeof(name), "ringbuffer_%dw_busyloop_1r_single", num_thread);
	run_ringbuffer(name, flags, args, num_thread, blocks);

	MUGGLE_LOG_INFO("=======================================================");
	flags = MUGGLE_RING_BUFFER_FLAG_WRITE_BUSY_LOOP | MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP;
	snprintf(name, sizeof(name), "ringbuffer_%dw_busyloop_1r_busyloop", num_thread);
	run_ringbuffer(name, flags, args, num_thread, blocks);

	// free memory
	free(args);
	free(blocks);

	return 0;
}
