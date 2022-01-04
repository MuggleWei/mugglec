#include "muggle_benchmark_mempool.h"

typedef struct muggle_benchmark_mempool_producer_args
{
	muggle_benchmark_mempool_t *benchmark; //!< benchmark
	muggle_ring_buffer_t       *ring;      //!< ring buffer
	int                        producer_id;
} muggle_benchmark_mempool_producer_args_t;

typedef struct muggle_benchmark_mempool_consumer_args
{
	muggle_benchmark_mempool_t *benchmark; //!< benchmark
	muggle_ring_buffer_t       *ring;      //!< ring buffer
	int                        consumer_id;
} muggle_benchmark_mempool_consumer_args_t;

static void muggle_benchmark_mempool_run_single_thread(muggle_benchmark_mempool_t *benchmark)
{
	muggle_benchmark_handle_t *handle = &benchmark->handle;
	muggle_benchmark_config_t *config = benchmark->config;

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

static muggle_thread_ret_t muggle_benchmark_mempool_consumer(void *p_args)
{
	muggle_benchmark_mempool_consumer_args_t *args = (muggle_benchmark_mempool_consumer_args_t*)p_args;

	muggle_benchmark_mempool_t *benchmark = args->benchmark;
	muggle_ring_buffer_t *ring = args->ring;

	muggle_benchmark_handle_t *handle = &benchmark->handle;
	muggle_benchmark_config_t *config = benchmark->config;

	fn_muggle_benchmark_record fn_record = muggle_benchmark_get_fn_record(config->elapsed_unit);

	muggle_benchmark_record_t *before_free_records =
		muggle_benchmark_handle_get_records(handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_FREE);
	muggle_benchmark_record_t *after_free_records =
		muggle_benchmark_handle_get_records(handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_FREE);

	fn_muggle_benchmark_mempool_free fn_free = benchmark->fn_free;
	void *pool = benchmark->pool;

	uint64_t count = 0;
	while (1)
	{
		void *data = muggle_ring_buffer_read(ring, 0);
		if (data == NULL)
		{
			break;
		}

		uint64_t idx = *(uint64_t*)data;

		fn_record(&before_free_records[idx]);
		fn_free(pool, data);
		fn_record(&after_free_records[idx]);

		++count;
	}

	MUGGLE_LOG_INFO("consumer[%d] completed, total read record: %llu",
		args->consumer_id, (unsigned long long)count);

	return 0;
}

static muggle_thread_ret_t muggle_benchmark_mempool_producer(void *p_args)
{
	muggle_benchmark_mempool_producer_args_t *args = (muggle_benchmark_mempool_producer_args_t*)p_args;

	muggle_benchmark_mempool_t *benchmark = args->benchmark;
	muggle_ring_buffer_t *ring = args->ring;

	muggle_benchmark_handle_t *handle = &benchmark->handle;
	muggle_benchmark_config_t *config = benchmark->config;

	fn_muggle_benchmark_record fn_record = muggle_benchmark_get_fn_record(config->elapsed_unit);

	muggle_benchmark_record_t *before_alloc_records =
		muggle_benchmark_handle_get_records(handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_ALLOC);
	muggle_benchmark_record_t *after_alloc_records =
		muggle_benchmark_handle_get_records(handle, MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_ALLOC);

	fn_muggle_benchmark_mempool_alloc fn_alloc = benchmark->fn_alloc;
	void *pool = benchmark->pool;

	uint64_t idx = 0;
	void *data = NULL;
	uint64_t count = config->rounds * config->record_per_round;
	uint64_t offset = args->producer_id * count;
	size_t block_size = config->block_size;

	for (uint64_t round = 0; round < config->rounds; round++)
	{
		for (uint64_t i = 0; i < config->record_per_round; i++)
		{
			uint64_t idx = offset + round * config->record_per_round + i;

			fn_record(&before_alloc_records[idx]);
			data = fn_alloc(pool, block_size);
			fn_record(&after_alloc_records[idx]);

			*(uint64_t*)data = idx;

			muggle_ring_buffer_write(ring, data);
		}

		if (config->round_interval_ms > 0)
		{
			muggle_msleep(config->round_interval_ms);
		}
	}

	MUGGLE_LOG_INFO("producer[%d] completed, total allocate %llu times", 
		args->producer_id, (unsigned long long)count);
	return 0;
}

static void muggle_benchmark_mempool_run_multi_thread(muggle_benchmark_mempool_t *benchmark)
{
	muggle_benchmark_config_t *config = benchmark->config;

	// prepare ring buffer
	muggle_ring_buffer_t ring;
	muggle_ring_buffer_init(&ring, config->capacity, MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE);

	// prepare producer
	muggle_thread_t *producer_threads = (muggle_thread_t*)malloc(sizeof(muggle_thread_t) * config->producer);
	if (producer_threads == NULL)
	{
		MUGGLE_LOG_ERROR("failed allocate producer thread");
		goto mempool_finally;
	}

	muggle_benchmark_mempool_producer_args_t *producer_args =
		(muggle_benchmark_mempool_producer_args_t*)malloc(
			sizeof(muggle_benchmark_mempool_producer_args_t) * config->producer);
	if (producer_args == NULL)
	{
		goto mempool_finally;
	}
	for (int i = 0; i < config->producer; i++)
	{
		memset(&producer_args[i], 0, sizeof(muggle_benchmark_mempool_producer_args_t));
		producer_args[i].benchmark = benchmark;
		producer_args[i].ring = &ring;
		producer_args[i].producer_id = i;
	}

	// prepare consumer
	muggle_thread_t *consumer_threads = (muggle_thread_t*)malloc(sizeof(muggle_thread_t) * config->consumer);
	if (consumer_threads == NULL)
	{
		MUGGLE_LOG_ERROR("failed allocate consumer thread");
		goto mempool_finally;
	}

	muggle_benchmark_mempool_consumer_args_t *consumer_args =
		(muggle_benchmark_mempool_consumer_args_t *)malloc(
			sizeof(muggle_benchmark_mempool_consumer_args_t) * config->consumer);
	if (consumer_args == NULL)
	{
		goto mempool_finally;
	}
	for (int i = 0; i < config->consumer; i++)
	{
		memset(&consumer_args[i], 0, sizeof(muggle_benchmark_mempool_consumer_args_t));
		consumer_args[i].benchmark = benchmark;
		consumer_args[i].ring = &ring;
		consumer_args[i].consumer_id = i;
	}

	// run consumers
	for (int i = 0; i < config->consumer; i++)
	{
		muggle_thread_create(&consumer_threads[i], muggle_benchmark_mempool_consumer, &consumer_args[i]);
	}

	// run producers
	for (int i = 0; i < config->producer; i++)
	{
		muggle_thread_create(&producer_threads[i], muggle_benchmark_mempool_producer, &producer_args[i]);
	}

	// join producers
	for (int i = 0; i < config->producer; i++)
	{
		muggle_thread_join(&producer_threads[i]);
	}

	// producer completed callback
	for (int i = 0; i < config->consumer; i++)
	{
		muggle_ring_buffer_write(&ring, NULL);
	}

	// join consumers
	for (int i = 0; i < config->consumer; i++)
	{
		muggle_thread_join(&consumer_threads[i]);
	}

mempool_finally:
	// free consumer arguments
	if (consumer_args)
	{
		free(consumer_args);
	}

	if (consumer_threads)
	{
		free(consumer_threads);
	}

	// free producer arguments
	if (producer_args)
	{
		free(producer_args);
	}

	if (producer_threads)
	{
		free(producer_threads);
	}

	muggle_ring_buffer_destroy(&ring);
}

int muggle_benchmark_mempool_init(
	muggle_benchmark_mempool_t *benchmark,
	muggle_benchmark_config_t *config,
	int support_mul_thread,
	void *pool,
	fn_muggle_benchmark_mempool_alloc fn_alloc,
	fn_muggle_benchmark_mempool_free  fn_free)
{
	if (config->block_size < sizeof(uint64_t))
	{
		MUGGLE_LOG_ERROR("benchmark will use data first 8 bytes, so block size at least need 8 byte");
		return -1;
	}

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
		muggle_benchmark_mempool_run_multi_thread(benchmark);
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
