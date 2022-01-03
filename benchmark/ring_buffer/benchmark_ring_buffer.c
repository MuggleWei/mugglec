#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"
#include <threads.h>

int ring_write(void *user_args, void *data)
{
	return muggle_ring_buffer_write((muggle_ring_buffer_t*)user_args, data);
}

thread_local int r_cursor = 0;
void* ring_read(void *user_args)
{
	return muggle_ring_buffer_read((muggle_ring_buffer_t*)user_args, r_cursor++);
}

void producer_complete_cb(muggle_benchmark_config_t *config, void *user_args)
{
	static muggle_benchmark_thread_message_t end_msg;
	memset(&end_msg, 0, sizeof(end_msg));
	end_msg.id = UINT64_MAX;
	for (int i = 0; i < config->consumer; i++)
	{
		ring_write(user_args, (void*)&end_msg);
		muggle_msleep(10);
	}
}

void benchmark_ring_buffer(muggle_benchmark_config_t *config, int flags, const char *name)
{
	// prepare ring buffer
	muggle_ring_buffer_t ring;
	muggle_ring_buffer_init(&ring, config->capacity, flags);

	// initialize thread transfer benchmark
	muggle_benchmark_thread_trans_t benchmark;
	muggle_benchmark_thread_trans_init(
		&benchmark,
		config,
		(void*)&ring,
		ring_write,
		ring_read,
		producer_complete_cb);

	// run benchmark
	muggle_benchmark_thread_trans_run(&benchmark);

	// generate report
	muggle_benchmark_thread_trans_gen_report(&benchmark, name);

	// destroy benchmark
	muggle_benchmark_thread_trans_destroy(&benchmark);

	// free ring buffer
	muggle_ring_buffer_destroy(&ring);
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

	// ring_buffer - lock write and wait read
	flags = MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK | MUGGLE_RING_BUFFER_FLAG_READ_WAIT;
	benchmark_ring_buffer(&config, flags, "ringbuffer_lock_wait");

	// ring_buffer - lock write and busy read
	flags = MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK | MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP;
	benchmark_ring_buffer(&config, flags, "ringbuffer_lock_busy");

	// ring_buffer - busy write and wait read
	flags = MUGGLE_RING_BUFFER_FLAG_WRITE_BUSY_LOOP | MUGGLE_RING_BUFFER_FLAG_READ_WAIT;
	benchmark_ring_buffer(&config, flags, "ringbuffer_busy_wait");

	// ring_buffer - busy write and busy read
	flags = MUGGLE_RING_BUFFER_FLAG_WRITE_BUSY_LOOP | MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP;
	benchmark_ring_buffer(&config, flags, "ringbuffer_busy_busy");
	

	return 0;
}
