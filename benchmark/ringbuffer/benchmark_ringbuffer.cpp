/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle_benchmark/muggle_benchmark.h"
#include <stdio.h>
#include <thread>
#include <chrono>

void fn_producer(
	muggle::BenchmarkConfig *config,
	muggle::LatencyBlock *blocks,
	muggle_ringbuffer_t *ring,
	muggle_atomic_int *consumer_ready,
	muggle_atomic_int consumer_cnt,
	uint64_t start_idx,
	uint64_t end_idx
)
{
	while (muggle_atomic_load(consumer_ready, muggle_memory_order_relaxed) != consumer_cnt);

	for (uint64_t i = 0; i < config->loop; ++i)
	{
		for (uint64_t j = start_idx; j < end_idx; ++j)
		{
			uint64_t idx = i * config->cnt_per_loop + j;
			memset(&blocks[idx], 0, sizeof(muggle::LatencyBlock));
			blocks[idx].idx = idx;

			timespec_get(&blocks[idx].ts[0], TIME_UTC);
			muggle_ringbuffer_push(ring, &blocks[idx], 1, 0);
			timespec_get(&blocks[idx].ts[1], TIME_UTC);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(config->loop_interval_ms));
	}
	muggle_ringbuffer_push(ring, nullptr, 1, 0);
}

void fn_consumer(
	muggle_ringbuffer_t *ring,
	muggle_atomic_int *consumer_ready,
	int print_msg_loss
)
{
	muggle_atomic_fetch_add(consumer_ready, 1, muggle_memory_order_relaxed);
	muggle_atomic_int pos = 0;
	uint64_t idx = 0;
	while (1)
	{
		muggle::LatencyBlock *block = (muggle::LatencyBlock*)muggle_ringbuffer_get(ring, pos++);
		if (!block)
		{
			break;
		}
		timespec_get(&block->ts[2], TIME_UTC);
		if (print_msg_loss && idx != block->idx)
		{
			printf("expect: %llu, actual: %llu\n", (unsigned long long)idx, (unsigned long long)block->idx);
			idx = block->idx;
		}
		++idx;
	}
}

void Benchmark_1w1r(FILE *fp, muggle::BenchmarkConfig &config)
{
	uint64_t cnt = config.loop * config.cnt_per_loop;
	muggle::LatencyBlock *blocks = (muggle::LatencyBlock*)malloc(cnt * sizeof(muggle::LatencyBlock));
	muggle_atomic_int consumer_ready = 0;

	muggle_ringbuffer_t ring;
	muggle_ringbuffer_init(&ring, 1024 * 16);

	std::thread producer(fn_producer,
		&config, blocks, &ring, &consumer_ready, 1, 0, config.cnt_per_loop);
	std::thread consumer(fn_consumer,
		&ring, &consumer_ready, 1);

	producer.join();
	consumer.join();

	muggle_ringbuffer_destroy(&ring);

	muggle::GenLatencyReportsHead(fp, &config);
	muggle::GenLatencyReportsBody(fp, &config, blocks, "1w1r-w", cnt, 0, 1, 0);
	muggle::GenLatencyReportsBody(fp, &config, blocks, "1w1r-w-sorted", cnt, 0, 1, 1);
	muggle::GenLatencyReportsBody(fp, &config, blocks, "1w1r-wr", cnt, 0, 2, 0);
	muggle::GenLatencyReportsBody(fp, &config, blocks, "1w1r-wr-sorted", cnt, 0, 2, 1);

	free(blocks);
}

int main()
{
	muggle::BenchmarkConfig config;
	strncpy(config.name, "ringbuffer", sizeof(config.name)-1);
	config.loop = 50;
	config.cnt_per_loop = 10000;
	config.loop_interval_ms = 1;
	config.report_step = 10;

	char file_name[128];
	snprintf(file_name, sizeof(file_name)-1, "benchmark_%s.csv", config.name);
	FILE *fp = fopen(file_name, "wb");
	if (fp == nullptr)
	{
		printf("failed open file: %s\n", file_name);
		exit(1);
	}

	// 1 writer, 1 reader
	Benchmark_1w1r(fp, config);

	fclose(fp);
}
