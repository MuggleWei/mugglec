#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

#define ASSERT_GE(x, y) if ((x) < (y)) printf("expect %d >= %d\n", (x), (y))
#define EXPECT_EQ(x, y) if ((x) != (y)) printf("expect %d == %d\n", (x), (y))

struct consumer_thread_args
{
	muggle_double_buffer_t *double_buffer;
	muggle_atomic_int *total_read;
	muggle_atomic_int *consumer_ready;
};

struct producer_thread_args
{
	int idx;
	muggle_double_buffer_t *double_buffer;
	muggle_benchmark_block_t *blocks;
	muggle_atomic_int *total_read;
	muggle_atomic_int *consumer_ready;
	int cnt_consumer;
	int total;
	muggle_atomic_int *fetch_id;
	int cnt_interval;
	int interval_ms;
};

void get_case_name(char *buf, size_t max_len, int cnt_producer, int cnt_consumer)
{
	snprintf(buf, max_len, "%dw-%dr", cnt_producer, cnt_consumer);
}

muggle_thread_ret_t consumer_thread(void *void_arg)
{
	struct consumer_thread_args *arg = (struct consumer_thread_args*)void_arg;

	muggle_atomic_int consumer_idx = muggle_atomic_fetch_add(arg->consumer_ready, 1, muggle_memory_order_relaxed);

	int cnt = 0;
	struct timespec start_ts, end_ts;
	timespec_get(&start_ts, TIME_UTC);
	int is_end = 0;
	while (1)
	{
		muggle_single_buffer_t *buf = muggle_double_buffer_read(arg->double_buffer);
		for (int i = 0; i < buf->cnt; ++i)
		{
			muggle_benchmark_block_t *block = (muggle_benchmark_block_t*)buf->datas[i];
			if (block == NULL)
			{
				is_end = 1;
				break;
			}
			timespec_get(&block->ts[2], TIME_UTC);
			++cnt;
		}
		if (is_end)
		{
			break;
		}
	}
	timespec_get(&end_ts, TIME_UTC);

	uint64_t elapsed_ns =
		(end_ts.tv_sec - start_ts.tv_sec) * 1000000000 +
		end_ts.tv_nsec - start_ts.tv_nsec;
	printf("consumer[%d] read %d messages, total use: %lldns(%.3fs)\n",
		consumer_idx, cnt, (unsigned long long)elapsed_ns, elapsed_ns / 1000000000.0);
	muggle_atomic_fetch_add(arg->total_read, cnt, muggle_memory_order_relaxed);

	return 0;
}

muggle_thread_ret_t producer_thread(void *void_arg)
{
	struct producer_thread_args *arg = (struct producer_thread_args*)void_arg;

	while (muggle_atomic_load(arg->consumer_ready, muggle_memory_order_acquire) != arg->cnt_consumer);

	muggle_atomic_int idx;
	int cnt = 0;
	struct timespec start_ts, end_ts;
	timespec_get(&start_ts, TIME_UTC);
	while (1)
	{
		idx = muggle_atomic_fetch_add(arg->fetch_id, 1, muggle_memory_order_relaxed);

		if (idx < arg->total)
		{
			timespec_get(&arg->blocks[idx].ts[0], TIME_UTC);
			muggle_double_buffer_write(arg->double_buffer, &arg->blocks[idx]);
			timespec_get(&arg->blocks[idx].ts[1], TIME_UTC);
		}
		else
		{
			break;
		}

		if (idx == arg->cnt_interval && arg->interval_ms > 0)
		{
			muggle_msleep(arg->interval_ms);
		}

		++cnt;
	}
	timespec_get(&end_ts, TIME_UTC);
	uint64_t elapsed_ns =
		(end_ts.tv_sec - start_ts.tv_sec) * 1000000000 +
		end_ts.tv_nsec - start_ts.tv_nsec;
	printf("producer[%d] write %d messages, total use: %lldns(%.3fs)\n",
		arg->idx, cnt, (unsigned long long)elapsed_ns, elapsed_ns / 1000000000.0);

	free(arg);

	return 0;
}

void producer_consumer(int capacity, int total, int cnt_producer, int cnt_interval, int interval_ms)
{
	int cnt_consumer = 1;
	muggle_double_buffer_t double_buffer;
	muggle_benchmark_block_t *blocks = (muggle_benchmark_block_t*)malloc(sizeof(muggle_benchmark_block_t) * total);
	for (int i = 0; i < total; ++i)
	{
		memset(&blocks[i], 0, sizeof(muggle_benchmark_block_t));
		blocks[i].idx = (uint64_t)i;
	}

	muggle_atomic_int total_read = 0;
	muggle_atomic_int consumer_ready = 0;
	muggle_double_buffer_init(&double_buffer, capacity, 0);
	char case_name[1024];
	get_case_name(case_name, sizeof(case_name)-1, cnt_producer, cnt_consumer);

	printf("================================\n");
	printf("launch %s\n", case_name);

	// consumer
	struct consumer_thread_args consumer_args;
	consumer_args.double_buffer = &double_buffer;
	consumer_args.total_read = &total_read;
	consumer_args.consumer_ready = &consumer_ready;
	muggle_thread_t *consumers = (muggle_thread_t*)malloc(cnt_consumer * sizeof(muggle_thread_t));
	for (int i = 0; i < cnt_consumer; ++i)
	{
		muggle_thread_create(&consumers[i], consumer_thread, &consumer_args);
	}

	// producer
	muggle_thread_t *producers = (muggle_thread_t*)malloc(cnt_producer * sizeof(muggle_thread_t));
	muggle_atomic_int fetch_id = 0;
	for (int i = 0; i < cnt_producer; ++i)
	{
		struct producer_thread_args *producer_args = (struct producer_thread_args*)malloc(sizeof(struct producer_thread_args));
		producer_args->idx = i;
		producer_args->double_buffer = &double_buffer;
		producer_args->blocks = blocks;
		producer_args->consumer_ready = &consumer_ready;
		producer_args->cnt_consumer = cnt_consumer;
		producer_args->total = total;
		producer_args->fetch_id = &fetch_id;
		producer_args->cnt_interval = cnt_interval;
		producer_args->interval_ms = interval_ms;
		muggle_thread_create(&producers[i], producer_thread, producer_args);
	}

	for (int i = 0; i < cnt_producer; ++i)
	{
		muggle_thread_join(&producers[i]);
	}

	for (int i = 0; i < cnt_consumer; ++i)
	{
		muggle_double_buffer_write(&double_buffer, NULL);
	}

	for (int i = 0; i < cnt_consumer; ++i)
	{
		muggle_thread_join(&consumers[i]);
	}

	free(producers);
	free(consumers);
	muggle_double_buffer_destroy(&double_buffer);

	// print elapsed
	uint64_t sum_write_ns = 0;
	uint64_t sum_trans_ns = 0;
	uint64_t total_write_ns = 0;
	uint64_t total_trans_ns = 0;
	uint64_t cnt = 0;
	for (int i = 0; i < total; i++)
	{
		sum_write_ns += 
			(blocks[i].ts[1].tv_sec - blocks[i].ts[0].tv_sec) * 1000000000 + 
			blocks[i].ts[1].tv_nsec - blocks[i].ts[0].tv_nsec;
		if (blocks[i].ts[2].tv_sec != 0)
		{
			sum_trans_ns +=
				(blocks[i].ts[2].tv_sec - blocks[i].ts[0].tv_sec) * 1000000000 + 
				blocks[i].ts[2].tv_nsec - blocks[i].ts[0].tv_nsec;
			cnt++;
		}
	}
	total_write_ns = 
		(blocks[total-1].ts[1].tv_sec - blocks[0].ts[0].tv_sec) * 1000000000 + 
		blocks[total-1].ts[1].tv_nsec - blocks[0].ts[0].tv_nsec;
	total_trans_ns = 
		(blocks[total-1].ts[2].tv_sec - blocks[0].ts[0].tv_sec) * 1000000000 + 
		blocks[total-1].ts[2].tv_nsec - blocks[0].ts[0].tv_nsec;

	printf("total read %d messages\n", total_read);
	printf("sum of write use %lluns(%.3fs), avg: %lluns\n",
		(unsigned long long)sum_write_ns, sum_write_ns / 1000000000.0, (unsigned long long)sum_write_ns / total);
	printf("sum of trans use %lluns(%.3fs), avg: %lluns\n",
		(unsigned long long)sum_trans_ns, sum_trans_ns / 1000000000.0, (unsigned long long)sum_trans_ns / cnt);
	printf("total of write use %lluns(%.3fs), avg: %lluns\n",
		(unsigned long long)total_write_ns, total_write_ns / 1000000000.0, (unsigned long long)total_write_ns / total);
	printf("total of trans use %lluns(%.3fs), avg: %lluns\n",
		(unsigned long long)total_trans_ns, total_trans_ns / 1000000000.0, (unsigned long long)total_trans_ns / cnt);

	free(blocks);
}

int main()
{
	int hc = muggle_thread_hardware_concurrency();
	hc /= 2;
	if (hc <= 1)
	{
		hc = 2;
	}

	int cnt_interval = 1000;
	int interval_ms = 0;
	int capacity = 1024 * 64;
	int total = 10000 * 100;

	// mul producer 1 consumer
	producer_consumer(capacity, total, hc, cnt_interval, interval_ms);

	return 0;
}
