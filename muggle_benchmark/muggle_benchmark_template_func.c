#include "muggle_benchmark_template_func.h"

typedef struct muggle_benchmark_func_thread_args
{
	muggle_benchmark_func_t *benchmark;
	int                     thread_id;
} muggle_benchmark_func_thread_args_t;

static muggle_thread_ret_t muggle_benchmark_func_thread(void *p_args)
{
	muggle_benchmark_func_thread_args_t *args =
		(muggle_benchmark_func_thread_args_t*)p_args;

	muggle_benchmark_func_t *benchmark = args->benchmark;

	muggle_benchmark_handle_t *handle = &benchmark->handle;
	muggle_benchmark_config_t *config = benchmark->config;

	fn_muggle_benchmark_record fn_record = muggle_benchmark_get_fn_record(config->elapsed_unit);
	fn_muggle_benchmark_func func = benchmark->func;

	muggle_benchmark_record_t *before_records =
		muggle_benchmark_handle_get_records(handle, MUGGLE_BENCHMARK_FUNC_ACTION_BEFORE);
	muggle_benchmark_record_t *after_records =
		muggle_benchmark_handle_get_records(handle, MUGGLE_BENCHMARK_FUNC_ACTION_AFTER);

	uint64_t idx = 0;
	uint64_t count = config->rounds * config->record_per_round;
	uint64_t offset = args->thread_id * count;
	for (uint64_t round = 0; round < config->rounds; round++)
	{
		for (uint64_t i = 0; i < config->record_per_round; i++)
		{
			uint64_t idx = offset + round * config->record_per_round + i;

			fn_record(&before_records[idx]);
			func(benchmark->args, idx);
			fn_record(&after_records[idx]);
		}

		if (config->round_interval_ms > 0)
		{
			muggle_msleep(config->round_interval_ms);
		}
	}

	MUGGLE_LOG_INFO("thread[%d] completed, total run %llu times", 
		args->thread_id, (unsigned long long)count);
	return 0;
}

int muggle_benchmark_func_init(
	muggle_benchmark_func_t *benchmark,
	muggle_benchmark_config_t *config,
	void *args,
	fn_muggle_benchmark_func func)
{
	memset(benchmark, 0, sizeof(*benchmark));
	benchmark->config = config;
	benchmark->args = args;
	benchmark->func = func;

	uint64_t record_count =
		config->rounds * config->record_per_round * config->producer;

	// initialize benchmark handle
	int ret = muggle_benchmark_handle_init(&benchmark->handle, record_count, MAX_MUGGLE_BENCHMARK_FUNC_ACTION);
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed initialize benchmark handle");
		return ret;
	}

	// set actions
	ret = muggle_benchmark_handle_set_action(
		&benchmark->handle, MUGGLE_BENCHMARK_FUNC_ACTION_BEFORE, "before");
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed set action: %d", MUGGLE_BENCHMARK_FUNC_ACTION_BEFORE);
		return ret;
	}

	ret = muggle_benchmark_handle_set_action(
		&benchmark->handle, MUGGLE_BENCHMARK_FUNC_ACTION_AFTER, "after");
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed set action: %d", MUGGLE_BENCHMARK_FUNC_ACTION_AFTER);
		return ret;
	}

	return 0;
}

void muggle_benchmark_func_destroy(muggle_benchmark_func_t *benchmark)
{
	// destroy benchmark handle
	muggle_benchmark_handle_destroy(&benchmark->handle);
}

void muggle_benchmark_func_run(muggle_benchmark_func_t *benchmark)
{
	muggle_benchmark_config_t *config = benchmark->config;

	muggle_thread_t *func_threads = NULL;
	muggle_benchmark_func_thread_args_t *thread_args = NULL;

	// prepare producer
	func_threads = (muggle_thread_t*)malloc(sizeof(muggle_thread_t) * config->producer);
	if (func_threads == NULL)
	{
		MUGGLE_LOG_ERROR("failed allocate thread");
		goto func_finally;
	}

	thread_args = (muggle_benchmark_func_thread_args_t*)malloc(
			sizeof(muggle_benchmark_func_thread_args_t) * config->producer);
	if (thread_args == NULL)
	{
		goto func_finally;
	}
	for (int i = 0; i < config->producer; i++)
	{
		memset(&thread_args[i], 0, sizeof(thread_args[i]));
		thread_args[i].benchmark = benchmark;
		thread_args[i].thread_id = i;
	}

	// run threads
	for (int i = 0; i < config->producer; i++)
	{
		muggle_thread_create(
			&func_threads[i], muggle_benchmark_func_thread, &thread_args[i]);
	}

	// join threads 
	for (int i = 0; i < config->producer; i++)
	{
		muggle_thread_join(&func_threads[i]);
	}

func_finally:
	if (func_threads)
	{
		free(func_threads);
	}

	if (thread_args)
	{
		free(thread_args);
	}
}

void muggle_benchmark_func_gen_report(muggle_benchmark_func_t *benchmark, const char *name)
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
		MUGGLE_BENCHMARK_FUNC_ACTION_BEFORE,
		MUGGLE_BENCHMARK_FUNC_ACTION_AFTER,
		0);
	muggle_benchmark_gen_latency_report_body(
		fp_latency, &benchmark->handle, benchmark->config,
		MUGGLE_BENCHMARK_FUNC_ACTION_BEFORE,
		MUGGLE_BENCHMARK_FUNC_ACTION_AFTER,
		1);
}
