#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

int pipe_write(void *user_args, void *data)
{
	return muggle_socket_evloop_pipe_write(
			   (muggle_socket_evloop_pipe_t *)user_args, data) ?
			   0 :
			   -1;
}

void *pipe_read(void *user_args, int consumer_id)
{
	MUGGLE_UNUSED(consumer_id);

	void *data = NULL;
	while ((data = muggle_socket_evloop_pipe_read(
				(muggle_socket_evloop_pipe_t *)user_args)) == NULL) {
	}
	return data;
}

void producer_complete_cb(muggle_benchmark_config_t *config, void *user_args)
{
	MUGGLE_UNUSED(config);

	static muggle_benchmark_thread_message_t end_msg;
	memset(&end_msg, 0, sizeof(end_msg));
	end_msg.id = UINT64_MAX;
	pipe_write(user_args, (void *)&end_msg);
}

void benchmark_ev_pipe(muggle_benchmark_config_t *config, const char *name)
{
	// prepare event pipe
	muggle_socket_evloop_pipe_t ev_pipe;
	muggle_socket_evloop_pipe_init(&ev_pipe);

	// initialize thread transfer benchmark
	muggle_benchmark_thread_trans_t benchmark;
	muggle_benchmark_thread_trans_init(&benchmark, config, (void *)&ev_pipe,
									   pipe_write, pipe_read,
									   producer_complete_cb);

	// run benchmark
	muggle_benchmark_thread_trans_run(&benchmark);

	// generate report
	muggle_benchmark_thread_trans_gen_report(&benchmark, name);

	// destroy benchmark
	muggle_benchmark_thread_trans_destroy(&benchmark);

	// free event pipe
	muggle_socket_evloop_pipe_destroy(&ev_pipe);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);

	config.producer = 0;

	muggle_benchmark_config_output(&config);

	// guarantee only one reader
	if (config.consumer != 1) {
		MUGGLE_LOG_ERROR("user must guarantee only one reader consume");
		exit(EXIT_FAILURE);
	}

	int hc = (int)muggle_thread_hardware_concurrency();
	if (hc <= 0) {
		hc = 2;
	}

	int producer_nums[] = { 1, 2, 4, hc / 2, hc, hc * 2, hc * 4 };

	char name[64];
	for (int i = 0; i < (int)(sizeof(producer_nums) / sizeof(producer_nums[0]));
		 i++) {
		int num_producer = producer_nums[i];
		memset(name, 0, sizeof(name));
		snprintf(name, sizeof(name), "event_pipe_%d", num_producer);

		config.producer = num_producer;

		MUGGLE_LOG_INFO(
			"--------------------------------------------------------");
		MUGGLE_LOG_INFO("run event pipe - %d write and 1 read", num_producer);
		benchmark_ev_pipe(&config, name);
	}

	return 0;
}
