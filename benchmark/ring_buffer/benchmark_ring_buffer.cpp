/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle_benchmark/muggle_benchmark.h"
#include <stdio.h>
#include <vector>
#include <thread>
#include <chrono>

uint64_t *consumer_read_num = nullptr;

void fn_producer(
	muggle_ring_buffer_t *ring,
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
			muggle_ring_buffer_write(ring, &blocks[idx]);
			timespec_get(&blocks[idx].ts[1], TIME_UTC);
		}
		if (config->loop_interval_ms > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(config->loop_interval_ms));
		}
	}
}

void fn_consumer(
	muggle_ring_buffer_t *ring,
	muggle_atomic_int *consumer_ready,
	int flag
)
{
	muggle_atomic_int consumer_idx = muggle_atomic_fetch_add(consumer_ready, 1, muggle_memory_order_relaxed);
	muggle_atomic_int pos = 0;
	uint64_t idx = 0;
	while (1)
	{
		muggle::LatencyBlock *block = (muggle::LatencyBlock*)muggle_ring_buffer_read(ring, pos++);
		if (!block)
		{
			break;
		}
		if (consumer_idx == 0 || (flag & MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE))
		{
			timespec_get(&block->ts[2], TIME_UTC);
		}
		++idx;
	}
	consumer_read_num[consumer_idx] = idx;
}

void get_case_name(char *buf, size_t max_len, int cnt_producer, int cnt_consumer, int w_mode, int r_mode)
{
	const char *str_w_mode[] = {
		"lock",
		"single",
		"busy_loop"
	};
	const char *str_r_mode[] = {
		"wait",
		"single_wait",
		"busy_loop",
		"lock"
	};

	snprintf(buf, max_len, "%dw%s-%dr%s",
		cnt_producer, str_w_mode[w_mode], 
		cnt_consumer, str_r_mode[r_mode]);
}

void Benchmark_wr(FILE *fp, muggle::BenchmarkConfig &config, int cnt_producer, int cnt_consumer, int flag)
{
	uint64_t cnt = config.loop * config.cnt_per_loop;
	muggle::LatencyBlock *blocks = (muggle::LatencyBlock*)malloc(cnt * sizeof(muggle::LatencyBlock));
	muggle_atomic_int consumer_ready = 0;

	consumer_read_num = (uint64_t*)malloc(sizeof(uint64_t) * cnt_consumer);

	muggle_ring_buffer_t ring;
	muggle_ring_buffer_init(&ring, 1024 * 16, flag);
	char case_name[1024];
	get_case_name(case_name, sizeof(case_name)-1, cnt_producer, cnt_consumer, ring.write_mode, ring.read_mode);

	printf("launch %s\n", case_name);

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
			&ring, &config, blocks, &consumer_ready, cnt_consumer, start_idx, end_idx
		));
	}
	for (int i = 0; i < cnt_consumer; ++i)
	{
		consumers.push_back(std::thread(fn_consumer,
			&ring, &consumer_ready, flag
		));
	}

	for (auto &producer : producers)
	{
		producer.join();
	}

	if (flag & MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE)
	{
		for (int i = 0; i < cnt_consumer; ++i)
		{
			muggle_ring_buffer_write(&ring, nullptr);
		}
	}
	else
	{
		muggle_ring_buffer_write(&ring, nullptr);
	}

	for (auto &consumer : consumers)
	{
		consumer.join();
	}

	muggle_ring_buffer_destroy(&ring);

	char buf[128];

	if (flag & MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE)
	{
		uint64_t cnt_consumer_reads = 0;
		for (int i = 0; i < cnt_consumer; ++i)
		{
			cnt_consumer_reads += consumer_read_num[i];
			printf("%s consumer[%d] read %llu\n",
				case_name, i, (unsigned long long)consumer_read_num[i]);
		}
		if (cnt_consumer_reads != config.cnt_per_loop * config.loop)
		{
			printf("%s\n", "message loss");
		}
	}
	else
	{
		for (int i = 0; i < cnt_consumer; ++i)
		{
			printf("%s consumer[%d] read %llu %s\n",
				case_name, i, (unsigned long long)consumer_read_num[i],
				consumer_read_num[i] == config.cnt_per_loop * config.loop ? "" : "(message loss)");
		}
	}
	free(consumer_read_num);

	snprintf(buf, sizeof(buf) - 1, "%s-w", case_name);
	muggle::GenLatencyReportsBody(fp, &config, blocks, buf, cnt, 0, 1, 0);

	snprintf(buf, sizeof(buf) - 1, "%s-w-sorted", case_name);
	muggle::GenLatencyReportsBody(fp, &config, blocks, buf, cnt, 0, 1, 1);

	snprintf(buf, sizeof(buf) - 1, "%s-wr", case_name);
	muggle::GenLatencyReportsBody(fp, &config, blocks, buf, cnt, 0, 2, 0);

	snprintf(buf, sizeof(buf) - 1, "%s-wr-sorted", case_name);
	muggle::GenLatencyReportsBody(fp, &config, blocks, buf, cnt, 0, 2, 1);

	free(blocks);
}

int main()
{
	muggle::BenchmarkConfig config;
	strncpy(config.name, "ring_buffer", sizeof(config.name)-1);
	config.loop = 50;
	config.cnt_per_loop = 20000;
	config.loop_interval_ms = 10;
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

	int hc = (int)std::thread::hardware_concurrency();
	if (hc <= 0)
	{
		hc = 2;
	}
	printf("hardware_concurrency: %d\n", hc);
	int hc_half = hc / 2;
	if (hc_half <= 1)
	{
		hc_half = 2;
	}

	int flag = 0;

	int w_flags[] = {
		MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK,
		MUGGLE_RING_BUFFER_FLAG_SINGLE_WRITER,
		MUGGLE_RING_BUFFER_FLAG_WRITE_BUSY_LOOP
	};
	int r_flags[] = {
		MUGGLE_RING_BUFFER_FLAG_READ_ALL | MUGGLE_RING_BUFFER_FLAG_READ_WAIT,
		MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP,
		MUGGLE_RING_BUFFER_FLAG_SINGLE_READER | MUGGLE_RING_BUFFER_FLAG_READ_WAIT,
		MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE 
	};

	for (int w_flag = 0; w_flag < (int)(sizeof(w_flags) / sizeof(w_flags[0])); ++w_flag)
	{
		for (int r_flag = 0; r_flag < (int)(sizeof(r_flags) / sizeof(r_flags[0])); ++r_flag)
		{
			flag = w_flags[w_flag] | r_flags[r_flag];

			Benchmark_wr(fp, config, 1, 1, flag);

			if (!(flag & MUGGLE_RING_BUFFER_FLAG_SINGLE_READER))
			{
				Benchmark_wr(fp, config, 1, hc_half, flag);
			}

			if (!(flag & MUGGLE_RING_BUFFER_FLAG_SINGLE_WRITER))
			{
				Benchmark_wr(fp, config, hc_half, 1, flag);
			}

			if (!(flag & MUGGLE_RING_BUFFER_FLAG_SINGLE_WRITER) &&
				!(flag & MUGGLE_RING_BUFFER_FLAG_SINGLE_READER))
			{
				Benchmark_wr(fp, config, hc_half, hc_half, flag);
			}
		}
	}

	fclose(fp);
}
