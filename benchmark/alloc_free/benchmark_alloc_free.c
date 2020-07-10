/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "alloc_free_runner.h"
#include "benchmark_malloc_free.h"
#include "benchmark_memory_pool.h"
#include "benchmark_sowr_memory_pool.h"
#include "benchmark_ts_memory_pool.h"

int main(int argc, char *argv[])
{
	// init log
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	int alloc_free_num = 100000;
	int mul_thread_pool_capacity = alloc_free_num / 20;

	muggle_benchmark_config_t benchmark_cfg;
	memset(&benchmark_cfg, 0, sizeof(benchmark_cfg));
	benchmark_cfg.loop = alloc_free_num;
	benchmark_cfg.loop_interval_ms = 0;
	benchmark_cfg.cnt_per_loop = 1;
	benchmark_cfg.report_step = 10;
	benchmark_cfg.elapsed_unit = MUGGLE_BENCHMARK_ELAPSED_UNIT_NS;

	// allocate memory
	muggle_benchmark_block_t *blocks = (muggle_benchmark_block_t*)malloc(sizeof(muggle_benchmark_block_t) * alloc_free_num);
	void **data_container = (void**)malloc(sizeof(void*) * alloc_free_num);

	// init args
	struct alloc_free_args args;
	memset(&args, 0, sizeof(args));
	args.cfg = &benchmark_cfg;
	args.blocks = blocks;
	args.data_container = data_container;

	char name[64];

	// benchmark alloc and free in same thread
	int data_size[] = {512, 1024, 2048, 4096};
	for (int i = 0; i < sizeof(data_size)/sizeof(data_size[0]); i++)
	{
		MUGGLE_LOG_INFO("=======================================================");
		snprintf(name, sizeof(name), "same_thread_malloc_free_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = NULL;
		args.data_size = data_size[i];
		args.cb_alloc = run_malloc;
		args.cb_free = run_free;
		args.num_alloc_threads = 0;
		args.num_free_threads = 0;
		run_alloc_free_benchmark(name, &args);

		MUGGLE_LOG_INFO("=======================================================");
		snprintf(name, sizeof(name), "malloc_1_free_1_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = NULL;
		args.data_size = data_size[i];
		args.cb_alloc = run_malloc;
		args.cb_free = run_free;
		args.num_alloc_threads = 1;
		args.num_free_threads = 1;
		run_alloc_free_benchmark(name, &args);

		MUGGLE_LOG_INFO("=======================================================");
		snprintf(name, sizeof(name), "malloc_2_free_1_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = NULL;
		args.data_size = data_size[i];
		args.cb_alloc = run_malloc;
		args.cb_free = run_free;
		args.num_alloc_threads = 2;
		args.num_free_threads = 1;
		run_alloc_free_benchmark(name, &args);

		MUGGLE_LOG_INFO("=======================================================");
		snprintf(name, sizeof(name), "malloc_4_free_1_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = NULL;
		args.data_size = data_size[i];
		args.cb_alloc = run_malloc;
		args.cb_free = run_free;
		args.num_alloc_threads = 4;
		args.num_free_threads = 1;
		run_alloc_free_benchmark(name, &args);

		MUGGLE_LOG_INFO("=======================================================");
		snprintf(name, sizeof(name), "malloc_2_free_2_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = NULL;
		args.data_size = data_size[i];
		args.cb_alloc = run_malloc;
		args.cb_free = run_free;
		args.num_alloc_threads = 2;
		args.num_free_threads = 2;
		run_alloc_free_benchmark(name, &args);

		MUGGLE_LOG_INFO("=======================================================");
		snprintf(name, sizeof(name), "malloc_4_free_4_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = NULL;
		args.data_size = data_size[i];
		args.cb_alloc = run_malloc;
		args.cb_free = run_free;
		args.num_alloc_threads = 4;
		args.num_free_threads = 4;
		run_alloc_free_benchmark(name, &args);
	}

	for (int i = 0; i < sizeof(data_size)/sizeof(data_size[0]); i++)
	{
		muggle_memory_pool_t pool;

		MUGGLE_LOG_INFO("=======================================================");
		muggle_memory_pool_init(&pool, alloc_free_num, data_size[i]);
		muggle_memory_pool_set_flag(&pool, MUGGLE_MEMORY_POOL_CONSTANT_SIZE);

		snprintf(name, sizeof(name), "same_thread_memory_pool_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = &pool;
		args.data_size = data_size[i];
		args.cb_alloc = run_memory_pool_alloc;
		args.cb_free = run_memory_pool_free;
		args.num_alloc_threads = 0;
		args.num_free_threads = 0;
		run_alloc_free_benchmark(name, &args);

		muggle_memory_pool_destroy(&pool);
	}

	for (int i = 0; i < sizeof(data_size)/sizeof(data_size[0]); i++)
	{
		muggle_sowr_memory_pool_t pool;

		MUGGLE_LOG_INFO("=======================================================");
		muggle_sowr_memory_pool_init(&pool, alloc_free_num, data_size[i]);

		snprintf(name, sizeof(name), "same_thread_sowr_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = &pool;
		args.data_size = data_size[i];
		args.cb_alloc = run_sowr_memory_pool_alloc;
		args.cb_free = run_sowr_memory_pool_free;
		args.num_alloc_threads = 0;
		args.num_free_threads = 0;
		run_alloc_free_benchmark(name, &args);

		muggle_sowr_memory_pool_destroy(&pool);


		MUGGLE_LOG_INFO("=======================================================");
		muggle_sowr_memory_pool_init(&pool, mul_thread_pool_capacity, data_size[i]);

		snprintf(name, sizeof(name), "sowr_alloc_1_free_1_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = &pool;
		args.data_size = data_size[i];
		args.cb_alloc = run_sowr_memory_pool_alloc;
		args.cb_free = run_sowr_memory_pool_free;
		args.num_alloc_threads = 1;
		args.num_free_threads = 1;
		run_alloc_free_benchmark(name, &args);

		muggle_sowr_memory_pool_destroy(&pool);
	}

	for (int i = 0; i < sizeof(data_size)/sizeof(data_size[0]); i++)
	{
		muggle_ts_memory_pool_t pool;

		MUGGLE_LOG_INFO("=======================================================");
		muggle_ts_memory_pool_init(&pool, alloc_free_num, data_size[i]);

		snprintf(name, sizeof(name), "same_thread_ts_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = &pool;
		args.data_size = data_size[i];
		args.cb_alloc = run_ts_memory_pool_alloc;
		args.cb_free = run_ts_memory_pool_free;
		args.num_alloc_threads = 0;
		args.num_free_threads = 0;
		run_alloc_free_benchmark(name, &args);

		muggle_ts_memory_pool_destroy(&pool);


		MUGGLE_LOG_INFO("=======================================================");
		muggle_ts_memory_pool_init(&pool, mul_thread_pool_capacity, data_size[i]);

		snprintf(name, sizeof(name), "ts_alloc_1_free_1_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = &pool;
		args.data_size = data_size[i];
		args.cb_alloc = run_ts_memory_pool_alloc;
		args.cb_free = run_ts_memory_pool_free;
		args.num_alloc_threads = 1;
		args.num_free_threads = 1;
		run_alloc_free_benchmark(name, &args);

		muggle_ts_memory_pool_destroy(&pool);


		MUGGLE_LOG_INFO("=======================================================");
		muggle_ts_memory_pool_init(&pool, mul_thread_pool_capacity, data_size[i]);

		snprintf(name, sizeof(name), "ts_alloc_2_free_1_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = &pool;
		args.data_size = data_size[i];
		args.cb_alloc = run_ts_memory_pool_alloc;
		args.cb_free = run_ts_memory_pool_free;
		args.num_alloc_threads = 2;
		args.num_free_threads = 1;
		run_alloc_free_benchmark(name, &args);

		muggle_ts_memory_pool_destroy(&pool);


		MUGGLE_LOG_INFO("=======================================================");
		muggle_ts_memory_pool_init(&pool, mul_thread_pool_capacity, data_size[i]);

		snprintf(name, sizeof(name), "ts_alloc_4_free_1_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = &pool;
		args.data_size = data_size[i];
		args.cb_alloc = run_ts_memory_pool_alloc;
		args.cb_free = run_ts_memory_pool_free;
		args.num_alloc_threads = 4;
		args.num_free_threads = 1;
		run_alloc_free_benchmark(name, &args);

		muggle_ts_memory_pool_destroy(&pool);


		MUGGLE_LOG_INFO("=======================================================");
		muggle_ts_memory_pool_init(&pool, mul_thread_pool_capacity, data_size[i]);

		snprintf(name, sizeof(name), "ts_alloc_2_free_2_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = &pool;
		args.data_size = data_size[i];
		args.cb_alloc = run_ts_memory_pool_alloc;
		args.cb_free = run_ts_memory_pool_free;
		args.num_alloc_threads = 2;
		args.num_free_threads = 2;
		run_alloc_free_benchmark(name, &args);

		muggle_ts_memory_pool_destroy(&pool);


		MUGGLE_LOG_INFO("=======================================================");
		muggle_ts_memory_pool_init(&pool, mul_thread_pool_capacity, data_size[i]);

		snprintf(name, sizeof(name), "ts_alloc_4_free_4_%d_%dbyte", alloc_free_num, data_size[i]);
		args.cnt_blocks = alloc_free_num;
		args.allocator = &pool;
		args.data_size = data_size[i];
		args.cb_alloc = run_ts_memory_pool_alloc;
		args.cb_free = run_ts_memory_pool_free;
		args.num_alloc_threads = 4;
		args.num_free_threads = 4;
		run_alloc_free_benchmark(name, &args);

		muggle_ts_memory_pool_destroy(&pool);
	}

	return 0;
}
