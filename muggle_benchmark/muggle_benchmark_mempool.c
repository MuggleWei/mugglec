#include "muggle_benchmark_mempool.h"

static void muggle_benchmark_mempool_run_single_thread(muggle_benchmark_mempool_t *benchmark)
{
	muggle_benchmark_handle_t *handle = &benchmark->handle;
	muggle_benchmark_config_t *config = benchmark->config;

	uint64_t record_count = muggle_benchmark_handle_get_record_count(handle);
	fn_muggle_benchmark_record fn_record = muggle_benchmark_get_fn_record(config->elapsed_unit);

	muggle_benchmark_record_t *before_alloc_records =
		muggle_benchmark_handle_get_records(handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_ALLOC);
	muggle_benchmark_record_t *after_alloc_records =
		muggle_benchmark_handle_get_records(handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_ALLOC);
	muggle_benchmark_record_t *before_free_records =
		muggle_benchmark_handle_get_records(handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_FREE);
	muggle_benchmark_record_t *after_free_records =
		muggle_benchmark_handle_get_records(handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_FREE);

	fn_muggle_benchmark_mempool_alloc fn_alloc = benchmark->fn_alloc;
	fn_muggle_benchmark_mempool_free fn_free = benchmark->fn_free;
	void *pool = benchmark->pool;

	size_t block_size = config->block_size;

	uint64_t idx = 0;
	void *data = NULL;
	for (uint64_t round = 0; round < config->rounds; round++)
	{
		for (uint64_t i = 0; i < config->record_per_round; i++)
		{
			fn_record(&before_alloc_records[idx]);
			data = fn_alloc(pool, block_size);
			fn_record(&after_alloc_records[idx]);

			fn_record(&before_free_records[idx]);
			fn_free(pool, data);
			fn_record(&after_free_records[idx]);

			idx++;
		}

		if (config->round_interval_ms > 0)
		{
			muggle_msleep(config->round_interval_ms);
		}
	}

	MUGGLE_LOG_INFO("total allocate and free %llu times", (unsigned long long)idx);
}

int muggle_benchmark_mempool_init(
	muggle_benchmark_mempool_t *benchmark,
	muggle_benchmark_config_t *config,
	int support_mul_thread,
	void *pool,
	fn_muggle_benchmark_mempool_alloc fn_alloc,
	fn_muggle_benchmark_mempool_free  fn_free)
{
	memset(benchmark, 0, sizeof(*benchmark));
	benchmark->config = config;
	benchmark->support_mul_thread = support_mul_thread;
	benchmark->pool = pool;
	benchmark->fn_alloc = fn_alloc;
	benchmark->fn_free = fn_free;

	uint64_t record_count =
		config->rounds * config->record_per_round * config->producer;

	// initialize benchmark handle
	int ret = muggle_benchmark_handle_init(&benchmark->handle, record_count, MAX_MUGGLE_BENCHMARK_MEMPOOL_ACTION);
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed initialize benchmark handle");
		return ret;
	}

	// set actions
	ret = muggle_benchmark_handle_set_action(
		&benchmark->handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_ALLOC, "before_alloc");
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed set action: %d", MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_ALLOC);
		return ret;
	}

	ret = muggle_benchmark_handle_set_action(
		&benchmark->handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_ALLOC, "after_alloc");
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed set action: %d", MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_ALLOC);
		return ret;
	}

	ret = muggle_benchmark_handle_set_action(
		&benchmark->handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_FREE, "before_free");
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed set action: %d", MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_FREE);
		return ret;
	}

	ret = muggle_benchmark_handle_set_action(
		&benchmark->handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_FREE, "after_free");
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed set action: %d", MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_FREE);
		return ret;
	}

	return 0;
}

void muggle_benchmark_mempool_destroy(muggle_benchmark_mempool_t *benchmark)
{
	// destroy benchmark handle
	muggle_benchmark_handle_destroy(&benchmark->handle);
}

void muggle_benchmark_mempool_run(muggle_benchmark_mempool_t *benchmark)
{
	if (benchmark->support_mul_thread)
	{
		// TODO:
	}
	else
	{
		muggle_benchmark_mempool_run_single_thread(benchmark);
	}
}

void muggle_benchmark_mempool_gen_report(muggle_benchmark_mempool_t *benchmark, const char *name)
{
	// generate records report
	char records_filepath[512];
	memset(records_filepath, 0, sizeof(records_filepath));
	snprintf(records_filepath, sizeof(records_filepath), "benchmark_%s_records.csv", name);
	FILE *fp_records = fopen(records_filepath, "wb");
	muggle_benchmark_handle_gen_records_report(&benchmark->handle, fp_records, benchmark->config->elapsed_unit);
	fclose(fp_records);

	// generate latency report
	char latency_filepath[512];
	memset(latency_filepath, 0, sizeof(latency_filepath));
	snprintf(latency_filepath, sizeof(latency_filepath), "benchmark_%s_latency.csv", name);
	FILE *fp_latency = fopen(latency_filepath, "wb");
	muggle_benchmark_gen_latency_report_head(fp_latency, benchmark->config);
	muggle_benchmark_gen_latency_report_body(
		fp_latency, &benchmark->handle, benchmark->config,
		MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_ALLOC,
		MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_ALLOC,
		0);
	muggle_benchmark_gen_latency_report_body(
		fp_latency, &benchmark->handle, benchmark->config,
		MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_ALLOC,
		MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_ALLOC,
		1);
	muggle_benchmark_gen_latency_report_body(
		fp_latency, &benchmark->handle, benchmark->config,
		MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_FREE,
		MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_FREE,
		0);
	muggle_benchmark_gen_latency_report_body(
		fp_latency, &benchmark->handle, benchmark->config,
		MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_FREE,
		MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_FREE,
		1);
	fclose(fp_latency);
}
