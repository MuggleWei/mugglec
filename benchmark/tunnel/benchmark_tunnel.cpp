/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle_benchmark/muggle_benchmark.h"
#include <queue>
#include <vector>
#include <thread>

void fn_producer(
	muggle::Tunnel<muggle::LatencyBlock*> *tunnel,
	muggle::BenchmarkConfig *config,
	muggle::LatencyBlock *blocks,
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
			tunnel->Write(&blocks[idx]);
			timespec_get(&blocks[idx].ts[1], TIME_UTC);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(config->loop_interval_ms));
	}
}

void fn_consumer(
	muggle::Tunnel<muggle::LatencyBlock*> *tunnel,
	muggle_atomic_int *consumer_ready
)
{
	muggle_atomic_fetch_add(consumer_ready, 1, muggle_memory_order_relaxed);
	std::queue<muggle::LatencyBlock*> queue;
	while (1)
	{
		tunnel->Read(queue);
		while (!queue.empty())
		{
			muggle::LatencyBlock *block = queue.front();
			if (!block)
			{
				return;
			}
			timespec_get(&block->ts[2], TIME_UTC);
			queue.pop();
		}
	}
}

void Benchmark_wr(FILE *fp, muggle::BenchmarkConfig &config, int cnt_producer, int cnt_consumer)
{
	uint64_t cnt = config.loop * config.cnt_per_loop;
	muggle::LatencyBlock *blocks = (muggle::LatencyBlock*)malloc(cnt * sizeof(muggle::LatencyBlock));
	muggle_atomic_int consumer_ready = 0;

	muggle::Tunnel<muggle::LatencyBlock*> tunnel;
	std::vector<std::thread> producers;
	std::vector<std::thread> consumers;

	for (int i = 0; i < cnt_producer; ++i)
	{
		uint64_t start_idx = i * (config.cnt_per_loop / cnt_producer);
		uint64_t end_idx = (i + 1) * (config.cnt_per_loop / cnt_producer);
		if (i == cnt_producer - 1)
		{
			end_idx = config.cnt_per_loop;
		}

		producers.push_back(std::thread(fn_producer,
			&tunnel, &config, blocks, &consumer_ready, cnt_consumer, start_idx, end_idx
		));
	}
	for (int i = 0; i < cnt_consumer; ++i)
	{
		consumers.push_back(std::thread(fn_consumer,
			&tunnel, &consumer_ready
		));
	}

	for (auto &producer : producers)
	{
		producer.join();
	}

	tunnel.Write(nullptr);

	for (auto &consumer : consumers)
	{
		consumer.join();
	}

	char buf[128];

	snprintf(buf, sizeof(buf) - 1, "%dw%dr-w", cnt_producer, cnt_consumer);
	muggle::GenLatencyReportsBody(fp, &config, blocks, buf, cnt, 0, 1, 0);

	snprintf(buf, sizeof(buf) - 1, "%dw%dr-w-sorted", cnt_producer, cnt_consumer);
	muggle::GenLatencyReportsBody(fp, &config, blocks, buf, cnt, 0, 1, 1);

	snprintf(buf, sizeof(buf) - 1, "%dw%dr-wr", cnt_producer, cnt_consumer);
	muggle::GenLatencyReportsBody(fp, &config, blocks, buf, cnt, 0, 2, 0);

	snprintf(buf, sizeof(buf) - 1, "%dw%dr-wr-sorted", cnt_producer, cnt_consumer);
	muggle::GenLatencyReportsBody(fp, &config, blocks, buf, cnt, 0, 2, 1);

	free(blocks);
}

int main()
{
	muggle::BenchmarkConfig config;
	strncpy(config.name, "tunnel", sizeof(config.name)-1);
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

	muggle::GenLatencyReportsHead(fp, &config);

	// 1 writer, 1 reader
	Benchmark_wr(fp, config, 1, 1);

	// 2 writer, 1 reader
	Benchmark_wr(fp, config, 2, 1);

	// 8 writer, 1 reader
	Benchmark_wr(fp, config, 8, 1);

	// 32 writer, 1 reader
	Benchmark_wr(fp, config, 32, 1);

	fclose(fp);
}
