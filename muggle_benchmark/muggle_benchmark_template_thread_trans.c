#include "muggle_benchmark_template_thread_trans.h"

/**
 * @brief thread trans benchmark producer arguments
 */
typedef struct muggle_benchmark_thread_producer_args
{
	fn_muggle_benchmark_thread_trans_write fn_write;  //!< write function
	fn_muggle_benchmark_record             fn_record; //!< record function
	muggle_benchmark_config_t              *config;
	muggle_benchmark_handle_t              *handle;
	muggle_benchmark_thread_message_t      *messages;
	int                                    producer_id;
	void                                   *user_args;
} muggle_benchmark_thread_producer_args_t;

/**
 * @brief thread trans benchmark consumer arguments
 */
typedef struct muggle_benchmark_thread_consumer_args
{
	fn_muggle_benchmark_thread_trans_read fn_read;     //!< read function
	fn_muggle_benchmark_record            fn_record;   //!< record function
	muggle_benchmark_handle_t             *handle;     //!< benchmark handle
	void                                  *user_args;  //!< user customized arguments
	int                                   consumer_id; //!< consumer id
} muggle_benchmark_thread_consumer_args_t;

static muggle_thread_ret_t muggle_benchmark_thread_trans_consumer(void *p_args)
{
	muggle_benchmark_thread_consumer_args_t *args = (muggle_benchmark_thread_consumer_args_t*)p_args;

	fn_muggle_benchmark_thread_trans_read fn_read = args->fn_read;
	fn_muggle_benchmark_record fn_record = args->fn_record;
	muggle_benchmark_record_t *records =
		muggle_benchmark_handle_get_records(args->handle, MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_READ);
	void *user_args = args->user_args;
	int consumer_id = args->consumer_id;
	uint64_t count = 0;
	while (1)
	{
		muggle_benchmark_thread_message_t *data = (muggle_benchmark_thread_message_t*)fn_read(user_args, consumer_id);
		if (!data || data->id == UINT64_MAX)
		{
			break;
		}

		muggle_benchmark_record_t *record = records + data->id;
		fn_record(record);
		record->idx = data->id;
		record->action = MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_READ;

		++count;
	}

	MUGGLE_LOG_INFO("consumer[%d] completed, total read record: %llu",
		args->consumer_id, (unsigned long long)count);

	return 0;
}

static muggle_thread_ret_t muggle_benchmark_thread_trans_producer(void *p_args)
{
	muggle_benchmark_thread_producer_args_t *args = (muggle_benchmark_thread_producer_args_t*)p_args;

	fn_muggle_benchmark_thread_trans_write fn_write = args->fn_write;
	fn_muggle_benchmark_record fn_record = args->fn_record;
	muggle_benchmark_config_t *config = args->config;
	void *user_args = args->user_args;
	muggle_benchmark_thread_message_t *messages = args->messages;

	muggle_benchmark_record_t *records_write_beg =
		muggle_benchmark_handle_get_records(args->handle, MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_BEG);
	muggle_benchmark_record_t *records_write_end =
		muggle_benchmark_handle_get_records(args->handle, MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_END);

	uint64_t count = config->rounds * config->record_per_round;
	uint64_t offset = args->producer_id * count;
	for (uint64_t round = 0; round < config->rounds; round++)
	{
		for (uint64_t i = 0; i < config->record_per_round; i++)
		{
			uint64_t idx = offset + round * config->record_per_round + i;
			muggle_benchmark_thread_message_t *data = &messages[idx];

			muggle_benchmark_record_t *record_write_beg = records_write_beg + idx;
			muggle_benchmark_record_t *record_write_end = records_write_end + idx;

			fn_record(record_write_beg);
			fn_write(user_args, data);
			fn_record(record_write_end);
		}

		if (config->round_interval_ns > 0)
		{
			muggle_nsleep(config->round_interval_ns);
		}
	}

	MUGGLE_LOG_INFO("producer[%d] completed, total write record: %llu",
		args->producer_id, (unsigned long long)count);

	return 0;
}


int muggle_benchmark_thread_trans_init(
	muggle_benchmark_thread_trans_t *benchmark,
	muggle_benchmark_config_t *config,
	void *user_args,
	fn_muggle_benchmark_thread_trans_write fn_write,
	fn_muggle_benchmark_thread_trans_read fn_read,
	fn_muggle_benchmark_thread_trans_producer_completed	fn_completed)
{
	memset(benchmark, 0, sizeof(*benchmark));
	benchmark->config = config;
	benchmark->user_args = user_args;

	uint64_t record_count =
		config->rounds * config->record_per_round * config->producer;

	// initialize benchmark handle
	int ret = muggle_benchmark_handle_init(&benchmark->handle, record_count, MAX_MUGGLE_BENCHMARK_THREAD_TRANS);
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed initialize benchmark handle");
		return ret;
	}

	// set actions
	ret = muggle_benchmark_handle_set_action(
		&benchmark->handle, MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_BEG, "write_begin");
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed set action: %d", MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_BEG);
		return ret;
	}

	ret = muggle_benchmark_handle_set_action(
		&benchmark->handle, MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_END, "write_end");
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed set action: %d", MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_END);
		return ret;
	}

	ret = muggle_benchmark_handle_set_action(
		&benchmark->handle, MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_READ, "read");
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed set action: %d", MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_READ);
		return ret;
	}

	// set callback function
	benchmark->fn_write = fn_write;
	benchmark->fn_read = fn_read;
	benchmark->fn_completed = fn_completed;

	return 0;
}

void muggle_benchmark_thread_trans_destroy(muggle_benchmark_thread_trans_t *benchmark)
{
	// destroy benchmark handle
	muggle_benchmark_handle_destroy(&benchmark->handle);
}

void muggle_benchmark_thread_trans_run(muggle_benchmark_thread_trans_t *benchmark)
{
	muggle_benchmark_handle_t *handle = &benchmark->handle;
	muggle_benchmark_config_t *config = benchmark->config;
	uint64_t record_count = muggle_benchmark_handle_get_record_count(handle);

	// prepare all messages and records
	muggle_benchmark_thread_message_t *messages = 
		(muggle_benchmark_thread_message_t*)malloc(sizeof(muggle_benchmark_thread_message_t) * record_count);
	if (messages == NULL)
	{
		MUGGLE_LOG_ERROR("failed allocate memory space for messages");
		goto thread_trans_finally;
	}
	for (uint64_t i = 0; i < record_count; i++)
	{
		messages[i].id = i;
	}

	// prepare producer
	muggle_thread_t *producer_threads = (muggle_thread_t*)malloc(sizeof(muggle_thread_t) * config->producer);
	if (producer_threads == NULL)
	{
		MUGGLE_LOG_ERROR("failed allocate producer thread");
		goto thread_trans_finally;
	}

	muggle_benchmark_thread_producer_args_t *producer_args =
		(muggle_benchmark_thread_producer_args_t *)malloc(
			sizeof(muggle_benchmark_thread_producer_args_t) * config->producer);
	if (producer_args == NULL)
	{
		goto thread_trans_finally;
	}
	for (int i = 0; i < config->producer; i++)
	{
		memset(&producer_args[i], 0, sizeof(muggle_benchmark_thread_producer_args_t));
		producer_args[i].fn_write = benchmark->fn_write;
		producer_args[i].fn_record = muggle_benchmark_get_fn_record(config->elapsed_unit);
		producer_args[i].config = config;
		producer_args[i].handle = handle;
		producer_args[i].messages = messages;
		producer_args[i].producer_id = i;
		producer_args[i].user_args = benchmark->user_args;
	}

	// prepare consumer
	muggle_thread_t *consumer_threads = (muggle_thread_t*)malloc(sizeof(muggle_thread_t) * config->consumer);
	if (consumer_threads == NULL)
	{
		MUGGLE_LOG_ERROR("failed allocate consumer thread");
		goto thread_trans_finally;
	}

	muggle_benchmark_thread_consumer_args_t *consumer_args =
		(muggle_benchmark_thread_consumer_args_t *)malloc(
			sizeof(muggle_benchmark_thread_consumer_args_t) * config->consumer);
	if (consumer_args == NULL)
	{
		goto thread_trans_finally;
	}
	for (int i = 0; i < config->consumer; i++)
	{
		memset(&consumer_args[i], 0, sizeof(muggle_benchmark_thread_consumer_args_t));
		consumer_args[i].fn_read = benchmark->fn_read;
		consumer_args[i].fn_record = muggle_benchmark_get_fn_record(config->elapsed_unit);
		consumer_args[i].handle = handle;
		consumer_args[i].user_args = benchmark->user_args;
		consumer_args[i].consumer_id = i;
	}

	// run consumers
	for (int i = 0; i < config->consumer; i++)
	{
		muggle_thread_create(&consumer_threads[i], muggle_benchmark_thread_trans_consumer, &consumer_args[i]);
	}

	// run producers
	for (int i = 0; i < config->producer; i++)
	{
		muggle_thread_create(&producer_threads[i], muggle_benchmark_thread_trans_producer, &producer_args[i]);
	}

	// join producers
	for (int i = 0; i < config->producer; i++)
	{
		muggle_thread_join(&producer_threads[i]);
	}

	// producer completed callback
	benchmark->fn_completed(config, benchmark->user_args);

	// join consumers
	for (int i = 0; i < config->consumer; i++)
	{
		muggle_thread_join(&consumer_threads[i]);
	}

thread_trans_finally:
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

	// free messages
	if (messages)
	{
		free(messages);
	}
}

void muggle_benchmark_thread_trans_gen_report(muggle_benchmark_thread_trans_t *benchmark, const char *name)
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
		MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_BEG,
		MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_END,
		0);
	muggle_benchmark_gen_latency_report_body(
		fp_latency, &benchmark->handle, benchmark->config,
		MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_BEG,
		MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_END,
		1);
	muggle_benchmark_gen_latency_report_body(
		fp_latency, &benchmark->handle, benchmark->config,
		MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_BEG,
		MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_READ,
		0);
	muggle_benchmark_gen_latency_report_body(
		fp_latency, &benchmark->handle, benchmark->config,
		MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_BEG,
		MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_READ,
		1);
	fclose(fp_latency);
}
