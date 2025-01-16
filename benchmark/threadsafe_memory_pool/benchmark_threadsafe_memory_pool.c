#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

void *threadsafe_memory_pool_alloc(void *pool, size_t block_size)
{
	MUGGLE_UNUSED(block_size);
	void *data = NULL;
	do {
		data = muggle_ts_memory_pool_alloc((muggle_ts_memory_pool_t *)pool);
	} while (data == NULL);
	return data;
}

void threadsafe_memory_pool_free(void *pool, void *data)
{
	MUGGLE_UNUSED(pool);
	muggle_ts_memory_pool_free(data);
}

void benchmark_threadsafe_memory_pool(muggle_benchmark_config_t *config,
									  const char *name)
{
	// initialize threadsafe memory pool
	muggle_ts_memory_pool_t pool;
	muggle_ts_memory_pool_init(&pool, config->capacity, config->block_size);

	// initialize benchmark memory pool handle
	muggle_benchmark_mempool_t benchmark;
	muggle_benchmark_mempool_init(&benchmark, config, 1, (void *)&pool,
								  threadsafe_memory_pool_alloc,
								  threadsafe_memory_pool_free);

	// run
	muggle_benchmark_mempool_run(&benchmark);

	// generate report
	muggle_benchmark_mempool_gen_report(&benchmark, name);

	// destroy benchmark memory pool handle
	muggle_benchmark_mempool_destroy(&benchmark);

	// destroy memory pool
	muggle_ts_memory_pool_destroy(&pool);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);

	config.block_size = 0;
	config.producer = 0;

	muggle_benchmark_config_output(&config);

	int hc = (int)muggle_thread_hardware_concurrency();
	if (hc <= 0) {
		hc = 2;
	}

	int producer_nums[] = { 1, 2, 4, hc / 2, hc, hc * 2, hc * 4 };

	char name[64];
	for (int i = 0; i < (int)(sizeof(producer_nums) / sizeof(producer_nums[0]));
		 i++) {
		int num_producer = producer_nums[i];

		MUGGLE_LOG_INFO(
			"--------------------------------------------------------");
		config.producer = num_producer;
		config.block_size = 128;
		memset(name, 0, sizeof(name));
		snprintf(name, sizeof(name), "ts_memory_pool_%d_w_128", num_producer);
		MUGGLE_LOG_INFO("run %s", name);
		benchmark_threadsafe_memory_pool(&config, name);

		MUGGLE_LOG_INFO(
			"--------------------------------------------------------");
		config.producer = num_producer;
		config.block_size = 256;
		memset(name, 0, sizeof(name));
		snprintf(name, sizeof(name), "ts_memory_pool_%d_w_256", num_producer);
		MUGGLE_LOG_INFO("run %s", name);
		benchmark_threadsafe_memory_pool(&config, name);

		MUGGLE_LOG_INFO(
			"--------------------------------------------------------");
		config.producer = num_producer;
		config.block_size = 512;
		memset(name, 0, sizeof(name));
		snprintf(name, sizeof(name), "ts_memory_pool_%d_w_512", num_producer);
		MUGGLE_LOG_INFO("run %s", name);
		benchmark_threadsafe_memory_pool(&config, name);

		MUGGLE_LOG_INFO(
			"--------------------------------------------------------");
		config.producer = num_producer;
		config.block_size = 1024;
		memset(name, 0, sizeof(name));
		snprintf(name, sizeof(name), "ts_memory_pool_%d_w_1024", num_producer);
		MUGGLE_LOG_INFO("run %s", name);
		benchmark_threadsafe_memory_pool(&config, name);

		MUGGLE_LOG_INFO(
			"--------------------------------------------------------");
		config.producer = num_producer;
		config.block_size = 4096;
		memset(name, 0, sizeof(name));
		snprintf(name, sizeof(name), "ts_memory_pool_%d_w_4096", num_producer);
		MUGGLE_LOG_INFO("run %s", name);
		benchmark_threadsafe_memory_pool(&config, name);
	}

	return 0;
}
