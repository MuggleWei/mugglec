#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

int array_blocking_queue_write(void *user_args, void *data)
{
	return muggle_array_blocking_queue_put((muggle_array_blocking_queue_t*)user_args, data);
}

void* array_blocking_queue_read(void *user_args, int consumer_id)
{
	return muggle_array_blocking_queue_take((muggle_array_blocking_queue_t*)user_args);
}

void producer_complete_cb(muggle_benchmark_config_t *config, void *user_args)
{
	static muggle_benchmark_thread_message_t end_msg;
	memset(&end_msg, 0, sizeof(end_msg));
	end_msg.id = UINT64_MAX;
	array_blocking_queue_write(user_args, (void*)&end_msg);
}

void benchmark_array_blocking_queue(muggle_benchmark_config_t *config, const char *name)
{
	// prepare array_blocking_queue
	muggle_array_blocking_queue_t queue;
	muggle_array_blocking_queue_init(&queue, config->capacity);

	// initialize thread transfer benchmark
	muggle_benchmark_thread_trans_t benchmark;
	muggle_benchmark_thread_trans_init(
		&benchmark,
		config,
		(void*)&queue,
		array_blocking_queue_write,
		array_blocking_queue_read,
		producer_complete_cb);

	// run benchmark
	muggle_benchmark_thread_trans_run(&benchmark);

	// generate report
	muggle_benchmark_thread_trans_gen_report(&benchmark, name);

	// destroy benchmark
	muggle_benchmark_thread_trans_destroy(&benchmark);

	// free array_blocking_queue
	muggle_array_blocking_queue_destroy(&queue);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);
	muggle_benchmark_config_output(&config);

	// array_blocking_queue - write and read
	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run array blocking queue");
	benchmark_array_blocking_queue(&config, "array_blocking_queue");

	return 0;
}
