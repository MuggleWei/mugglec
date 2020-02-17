#include <time.h>
#include <thread>
#include <chrono>
#include <vector>
#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

#define ASSERT_GE(x, y) if ((x) < (y)) printf("expect %d >= %d\n", (x), (y))
#define EXPECT_EQ(x, y) if ((x) != (y)) printf("expect %d == %d\n", (x), (y))

void get_case_name(char *buf, size_t max_len, int cnt_producer, int cnt_consumer)
{
	snprintf(buf, max_len, "%dw-%dr", cnt_producer, cnt_consumer);
}

void producer_consumer(int capacity, int total, int cnt_producer, int cnt_interval, int interval_ms)
{
	int cnt_consumer = 1;
	muggle_double_buffer_t double_buffer;
	muggle_benchmark_block *blocks = (muggle_benchmark_block*)malloc(sizeof(muggle_benchmark_block) * total);
	for (int i = 0; i < total; ++i)
	{
		memset(&blocks[i], 0, sizeof(muggle_benchmark_block));
		blocks[i].idx = (uint64_t)i;
	}

	muggle_atomic_int total_read = 0;
	muggle_atomic_int consumer_ready = 0;
	muggle_double_buffer_init(&double_buffer, capacity, 0);
	char case_name[1024];
	get_case_name(case_name, sizeof(case_name)-1, cnt_producer, cnt_consumer);

	printf("================================\n");
	printf("launch %s\n", case_name);

	std::vector<std::thread> consumers;
	for (int i = 0; i < cnt_consumer; ++i)
	{
		consumers.push_back(std::thread([&]{
			muggle_atomic_int consumer_idx = muggle_atomic_fetch_add(&consumer_ready, 1, muggle_memory_order_relaxed);

			int cnt = 0;
			struct timespec start_ts, end_ts;
			timespec_get(&start_ts, TIME_UTC);
			int is_end = 0;
			while (1)
			{
				muggle_single_buffer_t *buf = muggle_double_buffer_read(&double_buffer);
				for (int i = 0; i < buf->cnt; ++i)
				{
					muggle_benchmark_block *block = (muggle_benchmark_block*)buf->datas[i];
					if (block == nullptr)
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
			muggle_atomic_fetch_add(&total_read, cnt, muggle_memory_order_relaxed);
		}));
	}

	std::vector<std::thread> producers;
	muggle_atomic_int fetch_id = 0;
	for (int i = 0; i < cnt_producer; ++i)
	{
		producers.push_back(std::thread([&, i]{
			while (muggle_atomic_load(&consumer_ready, muggle_memory_order_acquire) != cnt_consumer);

			muggle_atomic_int idx;
			int cnt = 0;
			struct timespec start_ts, end_ts;
			timespec_get(&start_ts, TIME_UTC);
			while (1)
			{
				idx = muggle_atomic_fetch_add(&fetch_id, 1, muggle_memory_order_relaxed);

				if (idx < total)
				{
					timespec_get(&blocks[idx].ts[0], TIME_UTC);
					muggle_double_buffer_write(&double_buffer, &blocks[idx]);
					timespec_get(&blocks[idx].ts[1], TIME_UTC);
				}
				else
				{
					break;
				}

				if (idx == cnt_interval && interval_ms > 0)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
				}

				++cnt;
			}
			timespec_get(&end_ts, TIME_UTC);
			uint64_t elapsed_ns = 
				(end_ts.tv_sec - start_ts.tv_sec) * 1000000000 + 
				end_ts.tv_nsec - start_ts.tv_nsec;
			printf("producer[%d] write %d messages, total use: %lldns(%.3fs)\n",
				i, cnt, (unsigned long long)elapsed_ns, elapsed_ns / 1000000000.0);
		}));
	}

	for (auto &producer : producers)
	{
		producer.join();
	}

	for (int i = 0; i < cnt_consumer; ++i)
	{
		muggle_double_buffer_write(&double_buffer, nullptr);
	}

	for (auto &consumer : consumers)
	{
		consumer.join();
	}

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
	int hc = (int)std::thread::hardware_concurrency();
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
