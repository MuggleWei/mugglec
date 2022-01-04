#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

void* sowr_memory_pool_alloc(void *pool, size_t block_size)
{
	return muggle_sowr_memory_pool_alloc((muggle_sowr_memory_pool_t*)pool);
}

void sowr_memory_pool_free(void *pool, void *data)
{
	muggle_sowr_memory_pool_free(data);
}

void benchmark_sowr_memory_pool(muggle_benchmark_config_t *config, const char *name)
{
	// initialize sowr memory pool
	muggle_sowr_memory_pool_t pool;
	muggle_sowr_memory_pool_init(&pool, config->capacity, config->block_size);

	// initialize benchmark memory pool handle
	muggle_benchmark_mempool_t benchmark;
	muggle_benchmark_mempool_init(
		&benchmark,
		config,
		1,
		(void*)&pool,
		sowr_memory_pool_alloc,
		sowr_memory_pool_free);

	// run
	muggle_benchmark_mempool_run(&benchmark);

	// generate report
	muggle_benchmark_mempool_gen_report(&benchmark, name);

	// destroy benchmark memory pool handle
	muggle_benchmark_mempool_destroy(&benchmark);

	// destroy memory pool
	muggle_sowr_memory_pool_destroy(&pool);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);

	// force set producer 1 and consumer 1
	MUGGLE_LOG_INFO(
		"sowr memory pool only support 1 writer with 1 reader,"
		"force set producer and consumer");
	config.producer = 1;
	config.consumer = 1;
	config.block_size = 0;

	muggle_benchmark_config_output(&config);

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run sowr memory pool - block size: 128");
	config.block_size = 128;
	benchmark_sowr_memory_pool(&config, "sowr_memory_pool_128");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run sowr memory pool - block size: 256");
	config.block_size = 256;
	benchmark_sowr_memory_pool(&config, "sowr_memory_pool_256");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run sowr memory pool - block size: 512");
	config.block_size = 512;
	benchmark_sowr_memory_pool(&config, "sowr_memory_pool_512");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run sowr memory pool - block size: 1024");
	config.block_size = 1024;
	benchmark_sowr_memory_pool(&config, "sowr_memory_pool_1024");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run sowr memory pool - block size: 4096");
	config.block_size = 4096;
	benchmark_sowr_memory_pool(&config, "sowr_memory_pool_4096");

	return 0;
}
