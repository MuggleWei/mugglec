#include <time.h>
#include <thread>
#include <chrono>
#include <vector>
#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

#define ASSERT_GE(x, y) if ((x) < (y)) printf("expect %d >= %d\n", (x), (y))
#define EXPECT_EQ(x, y) if ((x) != (y)) printf("expect %d == %d\n", (x), (y))

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

void producer_consumer(int capacity, int flag, int total, int cnt_producer, int cnt_consumer, int cnt_interval, int interval_ms)
{
	muggle_ring_buffer_t r;
	muggle::LatencyBlock *blocks = (muggle::LatencyBlock*)malloc(sizeof(muggle::LatencyBlock) * total);
	for (int i = 0; i < total; ++i)
	{
		memset(&blocks[i], 0, sizeof(muggle::LatencyBlock));
		blocks[i].idx = (uint64_t)i;
	}

	muggle_atomic_int total_read = 0;
	muggle_atomic_int consumer_ready = 0;
	muggle_ring_buffer_init(&r, capacity, flag);
	char case_name[1024];
	get_case_name(case_name, sizeof(case_name)-1, cnt_producer, cnt_consumer, r.write_mode, r.read_mode);

	printf("================================\n");
	printf("launch %s\n", case_name);

	std::vector<std::thread> consumers;
	for (int i = 0; i < cnt_consumer; ++i)
	{
		consumers.push_back(std::thread([&]{
			muggle_atomic_int consumer_idx = muggle_atomic_fetch_add(&consumer_ready, 1, muggle_memory_order_relaxed);

			muggle_atomic_int pos = 0;
			uint64_t recv_idx = 0;
			int cnt = 0;
			struct timespec start_ts, end_ts;
			timespec_get(&start_ts, TIME_UTC);
			while (1)
			{
				muggle::LatencyBlock *block = (muggle::LatencyBlock*)muggle_ring_buffer_read(&r, pos++);
				if (block == nullptr)
				{
					break;
				}
				if (consumer_idx == 0 || (flag & MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE))
				{
					timespec_get(&block->ts[2], TIME_UTC);
				}

				if (cnt_producer == 1)
				{
					if (flag & MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE)
					{
						ASSERT_GE((int)block->idx, (int)recv_idx);
					}
					else
					{
						EXPECT_EQ((int)block->idx, (int)recv_idx);
					}
				}

				if (block->idx != recv_idx)
				{
					recv_idx = block->idx;
				}
				++recv_idx;
				++cnt;
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
					muggle_ring_buffer_write(&r, &blocks[idx]);
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

	if (flag & MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE)
	{
		for (int i = 0; i < cnt_consumer; ++i)
		{
			muggle_ring_buffer_write(&r, nullptr);
		}
	}
	else
	{
		muggle_ring_buffer_write(&r, nullptr);
	}

	for (auto &consumer : consumers)
	{
		consumer.join();
	}

	muggle_ring_buffer_destroy(&r);

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
	int flag = 0;
	int capacity = 1024 * 64;
	int total = 10000 * 100;

	// mul producer 1 consumer
	flag = 
		MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK |
		MUGGLE_RING_BUFFER_FLAG_SINGLE_READER |
		MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP;
	producer_consumer(capacity, flag, total, hc, 1, cnt_interval, interval_ms);

	flag = 
		MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK |
		MUGGLE_RING_BUFFER_FLAG_SINGLE_READER;
	producer_consumer(capacity, flag, total, hc, 1, cnt_interval, interval_ms);

	// 1 producer, mul consumer
	flag = 
		MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK |
		MUGGLE_RING_BUFFER_FLAG_READ_BUSY_LOOP;
	producer_consumer(capacity, flag, total, 1, hc, cnt_interval, interval_ms);

	flag = 
		MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK;
	producer_consumer(capacity, flag, total, 1, hc, cnt_interval, interval_ms);

	flag = 
		MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK |
		MUGGLE_RING_BUFFER_FLAG_MSG_READ_ONCE;
	producer_consumer(capacity, flag, total, 1, hc, cnt_interval, interval_ms);

	return 0;
}
