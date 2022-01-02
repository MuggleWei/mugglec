#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

// actions
enum
{
	ACTION_WRITE_BEG,
	ACTION_WRITE_END,
	ACTION_READ,
	MAX_ACTION
};

typedef struct message
{
	uint64_t id;
} message_t;

// thread arguments
typedef struct producer_args
{
	muggle_benchmark_config_t *config;
	muggle_benchmark_handle_t *handle;
	muggle_channel_t *chan;
	message_t *messages;
	int producer_idx;
} producer_args_t;

typedef struct consumer_args
{
	muggle_benchmark_handle_t *handle;
	muggle_channel_t *chan;
} consumer_args_t;

// end message
muggle_benchmark_thread_message_t end_msg;

int chan_write(void *user_args, void *data)
{
	return muggle_channel_write((muggle_channel_t*)user_args, data);
}

void* chan_read(void *user_args)
{
	return muggle_channel_read((muggle_channel_t*)user_args);
}

void producer_complete_cb(muggle_benchmark_config_t *config, void *user_args)
{
	memset(&end_msg, 0, sizeof(end_msg));
	end_msg.id = UINT64_MAX;
	chan_write(user_args, (void*)&end_msg);
}

void benchmark_chan(muggle_benchmark_config_t *config, int flags, const char *name)
{
	// prepare channel
	muggle_channel_t chan;
	muggle_channel_init(&chan, config->capacity, flags);

	// initialize thread transfer benchmark
	muggle_benchmark_thread_trans_t benchmark;
	muggle_benchmark_thread_trans_init(
		&benchmark,
		config,
		(void*)&chan,
		chan_write,
		chan_read,
		producer_complete_cb);

	// run benchmark
	muggle_benchmark_thread_trans_run(&benchmark);

	// generate report
	muggle_benchmark_thread_trans_gen_report(&benchmark, name);

	// destroy benchmark
	muggle_benchmark_thread_trans_destroy(&benchmark);

	// free channel
	muggle_channel_destroy(&chan);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);
	muggle_benchmark_config_output(&config);

	int flags = 0;

	// channel - futex write and wait read
	flags = MUGGLE_CHANNEL_FLAG_WRITE_FUTEX | MUGGLE_CHANNEL_FLAG_READ_WAIT;
	benchmark_chan(&config, flags, "channel_futex_wait");

	// channel - futex write and busy loop read
	flags = MUGGLE_CHANNEL_FLAG_WRITE_FUTEX | MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP;
	benchmark_chan(&config, flags, "channel_futex_busy");

	// channel - mutex write and wait read
	flags = MUGGLE_CHANNEL_FLAG_WRITE_MUTEX | MUGGLE_CHANNEL_FLAG_READ_WAIT;
	benchmark_chan(&config, flags, "channel_mutex_wait");

	// channel - mutex write and busy loop read
	flags = MUGGLE_CHANNEL_FLAG_WRITE_MUTEX | MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP;
	benchmark_chan(&config, flags, "channel_mutex_busy");

	return 0;
}
