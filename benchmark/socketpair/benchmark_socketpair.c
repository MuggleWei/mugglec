#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

typedef struct {
	muggle_spinlock_t write_spin;
	muggle_socket_t fds[2];
} socket_pair_args_t;

enum {
	SOCKET_PAIR_WRITER = 0,
	SOCKET_PAIR_READER = 1,
};

int socketpair_write(void *user_args, void *data)
{
	socket_pair_args_t *args = (socket_pair_args_t *)user_args;

	muggle_spinlock_lock(&args->write_spin);

	muggle_atomic_thread_fence(muggle_memory_order_release);
	muggle_socket_write(args->fds[SOCKET_PAIR_WRITER], &data, sizeof(void *));

	muggle_spinlock_unlock(&args->write_spin);

	return 0;
}

void *socketpair_read(void *user_args, int consumer_id)
{
	MUGGLE_UNUSED(consumer_id);

	socket_pair_args_t *args = (socket_pair_args_t *)user_args;

	void *data = NULL;
	muggle_socket_read(args->fds[SOCKET_PAIR_READER], &data, sizeof(void *));
	muggle_atomic_thread_fence(muggle_memory_order_acquire);

	return data;
}

void producer_complete_cb(muggle_benchmark_config_t *config, void *user_args)
{
	MUGGLE_UNUSED(config);

	static muggle_benchmark_thread_message_t end_msg;
	memset(&end_msg, 0, sizeof(end_msg));
	end_msg.id = UINT64_MAX;
	socketpair_write(user_args, (void *)&end_msg);
}

void benchmark_socketpair(muggle_benchmark_config_t *config, const char *name)
{
	// prepare socketpair
	socket_pair_args_t args;
	muggle_spinlock_init(&args.write_spin);
	if (muggle_socketpair(AF_UNIX, SOCK_DGRAM, 0, args.fds) != 0) {
		LOG_ERROR("failed init socketpair");
		return;
	}

	// initialize thread transfer benchmark
	muggle_benchmark_thread_trans_t benchmark;
	muggle_benchmark_thread_trans_init(&benchmark, config, (void *)&args,
									   socketpair_write, socketpair_read,
									   producer_complete_cb);

	// run benchmark
	muggle_benchmark_thread_trans_run(&benchmark);

	// generate report
	muggle_benchmark_thread_trans_gen_report(&benchmark, name);

	// destroy benchmark
	muggle_benchmark_thread_trans_destroy(&benchmark);

	// close socket pair
	muggle_socket_close(args.fds[0]);
	muggle_socket_close(args.fds[1]);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize socket lib
	if (muggle_socket_lib_init() != 0) {
		LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

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
		snprintf(name, sizeof(name), "socketpair_%d", num_producer);

		config.producer = num_producer;

		MUGGLE_LOG_INFO(
			"--------------------------------------------------------");
		MUGGLE_LOG_INFO("run socketpair - %d write and 1 read", num_producer);
		benchmark_socketpair(&config, name);
	}

	return 0;
}
