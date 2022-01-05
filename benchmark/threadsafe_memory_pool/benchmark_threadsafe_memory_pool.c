#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

void* threadsafe_memory_pool_alloc(void *pool, size_t block_size)
{
	return muggle_ts_memory_pool_alloc((muggle_ts_memory_pool_t*)pool);
}

void threadsafe_memory_pool_free(void *pool, void *data)
{
	muggle_ts_memory_pool_free(data);
}

void benchmark_threadsafe_memory_pool(muggle_benchmark_config_t *config, const char *name)
{
	// initialize threadsafe memory pool
	muggle_ts_memory_pool_t pool;
	muggle_ts_memory_pool_init(&pool, config->capacity, config->block_size);

	// initialize benchmark memory pool handle
	muggle_benchmark_mempool_t benchmark;
	muggle_benchmark_mempool_init(
		&benchmark,
		config,
		1,
		(void*)&pool,
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

	muggle_benchmark_config_output(&config);

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run threadsafe memory pool - block size: 128");
	config.block_size = 128;
	benchmark_threadsafe_memory_pool(&config, "threadsafe_memory_pool_128");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run threadsafe memory pool - block size: 256");
	config.block_size = 256;
	benchmark_threadsafe_memory_pool(&config, "threadsafe_memory_pool_256");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run threadsafe memory pool - block size: 512");
	config.block_size = 512;
	benchmark_threadsafe_memory_pool(&config, "threadsafe_memory_pool_512");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run threadsafe memory pool - block size: 1024");
	config.block_size = 1024;
	benchmark_threadsafe_memory_pool(&config, "threadsafe_memory_pool_1024");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run threadsafe memory pool - block size: 4096");
	config.block_size = 4096;
	benchmark_threadsafe_memory_pool(&config, "threadsafe_memory_pool_4096");

	return 0;
}

