/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle_benchmark/muggle_benchmark.h"

uint64_t *consumer_read_num = NULL;

struct consumer_thread_args
{
	muggle_ring_buffer_t *ring;
	muggle_atomic_int *consumer_ready;
	muggle_benchmark_block_t *blocks;
};

struct producer_thread_args
{
	muggle_ring_buffer_t *ring;
	muggle_benchmark_config_t *config;
	muggle_benchmark_block_t *blocks;
	muggle_atomic_int *consumer_ready;
	int cnt_consumer;
	uint64_t start_idx;
	uint64_t end_idx;
};

muggle_thread_ret_t consumer_thread(void *void_arg)
{
	struct consumer_thread_args *arg = (struct consumer_thread_args*)void_arg;

	muggle_atomic_int consumer_idx = muggle_atomic_fetch_add(arg->consumer_ready, 1, muggle_memory_order_relaxed);
	muggle_atomic_int pos = 0;
	uint64_t cnt = 0;
	while (1)
	{
		muggle_benchmark_block_t *block = (muggle_benchmark_block_t*)muggle_ring_buffer_read(arg->ring, pos++);
		if (!block)
		{
			break;
		}

		uint64_t block_idx = block->idx;
		timespec_get(&arg->blocks[block_idx].ts[2], TIME_UTC);
		muggle_sowr_memory_pool_free(block);
		timespec_get(&arg->blocks[block_idx].ts[3], TIME_UTC);

		++cnt;
	}
	consumer_read_num[consumer_idx] = cnt;

	return 0;
}

muggle_thread_ret_t producer_thread(void *void_arg)
{
	struct producer_thread_args *arg = (struct producer_thread_args*)void_arg;

	while (muggle_atomic_load(arg->consumer_ready, muggle_memory_order_relaxed) != arg->cnt_consumer);

	muggle_sowr_memory_pool_t pool;
	muggle_sowr_memory_pool_init(&pool, (muggle_atomic_int)(arg->config->loop * (arg->end_idx - arg->start_idx) / 10), sizeof(muggle_benchmark_block_t));

	for (uint64_t i = 0; i < arg->config->loop; ++i)
	{
		for (uint64_t j = arg->start_idx; j < arg->end_idx; ++j)
		{
			uint64_t idx = i * arg->config->cnt_per_loop + j;
			memset(&arg->blocks[idx], 0, sizeof(muggle_benchmark_block_t));
			arg->blocks[idx].idx = idx;

			timespec_get(&arg->blocks[idx].ts[0], TIME_UTC);
			muggle_benchmark_block_t *block = (muggle_benchmark_block_t*)muggle_sowr_memory_pool_alloc(&pool);
			timespec_get(&arg->blocks[idx].ts[1], TIME_UTC);
			if (block == NULL)
			{
				printf("failed allocate memory for idx: %llu\n", (unsigned long long)idx);
				continue;
			}

			block->idx = idx;
			muggle_ring_buffer_write(arg->ring, block);
		}
		if (arg->config->loop_interval_ms > 0)
		{
			muggle_msleep((unsigned int)arg->config->loop_interval_ms);
		}
	}

	while (!muggle_sowr_memory_pool_is_all_free(&pool))
	{
		muggle_msleep((unsigned int)1);
	}
	muggle_sowr_memory_pool_destroy(&pool);

	free(arg);
	return 0;
}

void Benchmark_wr(FILE *fp, muggle_benchmark_config_t *config, int cnt_producer, int cnt_consumer, int flag)
{
	uint64_t cnt = config->loop * config->cnt_per_loop;
	muggle_benchmark_block_t *blocks = (muggle_benchmark_block_t*)malloc(cnt * sizeof(muggle_benchmark_block_t));
	muggle_atomic_int consumer_ready = 0;

	consumer_read_num = (uint64_t*)malloc(sizeof(uint64_t) * cnt_consumer);

	muggle_ring_buffer_t ring;
	muggle_ring_buffer_init(&ring, 1024 * 16, flag);

	// consumer
	muggle_thread_t *consumers = (muggle_thread_t*)malloc(cnt_consumer * sizeof(muggle_thread_t));
	struct consumer_thread_args consumer_args;
	consumer_args.ring = &ring;
	consumer_args.consumer_ready = &consumer_ready;
	consumer_args.blocks = blocks;
	for (int i = 0; i < cnt_consumer; ++i)
	{
		muggle_thread_create(&consumers[i], consumer_thread, &consumer_args);
	}

	// producer
	muggle_thread_t *producers = (muggle_thread_t*)malloc(cnt_producer * sizeof(muggle_thread_t));
	for (int i = 0; i < cnt_producer; ++i)
	{
		struct producer_thread_args *producer_args = (struct producer_thread_args*)malloc(sizeof(struct producer_thread_args));
		producer_args->ring = &ring;
		producer_args->config = config;
		producer_args->blocks = blocks;
		producer_args->consumer_ready = &consumer_ready;
		producer_args->cnt_consumer = cnt_consumer;
		producer_args->start_idx = i * (config->cnt_per_loop / cnt_producer);
		producer_args->end_idx = (i + 1) * (config->cnt_per_loop / cnt_producer);
		if (i == cnt_producer - 1)
		{
			producer_args->end_idx = config->cnt_per_loop;
		}
		muggle_thread_create(&producers[i], producer_thread, producer_args);
	}
	
	for (int i = 0; i < cnt_producer; ++i)
	{
		muggle_thread_join(&producers[i]);
	}

	muggle_ring_buffer_write(&ring, NULL);

	for (int i = 0; i < cnt_consumer; ++i)
	{
		muggle_thread_join(&consumers[i]);
	}

	free(producers);
	free(consumers);
	muggle_ring_buffer_destroy(&ring);

	char buf[128];

	for (int i = 0; i < cnt_consumer; ++i)
	{
		printf("%dw%dr consumer[%d] read %llu %s\n",
			cnt_producer, cnt_consumer, i, (unsigned long long)consumer_read_num[i],
			consumer_read_num[i] == config->cnt_per_loop * config->loop ? "" : "(message loss)");
	}
	free(consumer_read_num);

	snprintf(buf, sizeof(buf) - 1, "%dw%dr-sowr-alloc", cnt_producer, cnt_consumer);
	muggle_benchmark_gen_reports_body(fp, config, blocks, buf, cnt, 0, 1, 0);

	snprintf(buf, sizeof(buf) - 1, "%dw%dr-sowr-alloc-sorted", cnt_producer, cnt_consumer);
	muggle_benchmark_gen_reports_body(fp, config, blocks, buf, cnt, 0, 1, 1);

	snprintf(buf, sizeof(buf) - 1, "%dw%dr-sowr-free", cnt_producer, cnt_consumer);
	muggle_benchmark_gen_reports_body(fp, config, blocks, buf, cnt, 2, 3, 0);

	snprintf(buf, sizeof(buf) - 1, "%dw%dr-sowr-free-sorted", cnt_producer, cnt_consumer);
	muggle_benchmark_gen_reports_body(fp, config, blocks, buf, cnt, 2, 3, 1);

	free(blocks);
}

int main()
{
	muggle_benchmark_config_t config;
	strncpy(config.name, "sowr_memory_pool", sizeof(config.name)-1);
	config.loop = 50;
	config.cnt_per_loop = 20000;
	config.loop_interval_ms = 10;
	config.report_step = 10;
	config.elapsed_unit = MUGGLE_BENCHMARK_ELAPSED_UNIT_NS;

	char file_name[128];
	snprintf(file_name, sizeof(file_name)-1, "benchmark_%s.csv", config.name);
	FILE *fp = fopen(file_name, "wb");
	if (fp == NULL)
	{
		printf("failed open file: %s\n", file_name);
		exit(1);
	}

	muggle_benchmark_gen_reports_head(fp, &config);

	int hc = (int)muggle_thread_hardware_concurrency();
	printf("hardware_concurrency: %d\n", hc);

	int flag = 0;

	// 1 writer, 1 reader
	Benchmark_wr(fp, &config, 1, 1, flag);

	// hc write, 1 reader
	Benchmark_wr(fp, &config, hc, 1, flag);

	// 2 * hc write, 1 reader
	Benchmark_wr(fp, &config, 2 * hc, 1, flag);

	fclose(fp);
}
