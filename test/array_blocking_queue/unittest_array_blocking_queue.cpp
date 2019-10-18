#include <thread>
#include <chrono>
#include <vector>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

int g_cnt_interval = 1024 * 2;
int g_interval_ms = 1;

TEST(array_blocking_queue, init_destroy)
{
	muggle_array_blocking_queue_t queue;
	int capacity = 16;
	int ret;

	ret = muggle_array_blocking_queue_init(&queue, capacity);
	EXPECT_EQ(ret, MUGGLE_OK);
	EXPECT_EQ(queue.capacity, capacity);

	ret = muggle_array_blocking_queue_destroy(&queue);
	EXPECT_EQ(ret, MUGGLE_OK);
}

TEST(array_blocking_queue, write_read_in_single_thread)
{
	muggle_array_blocking_queue_t queue;
	int capacity = 16;
	int arr[160];

	muggle_array_blocking_queue_init(&queue, capacity);

	// push capacity / 2, and get all
	for (muggle_atomic_int i = 0; i < capacity / 2; ++i)
	{
		arr[i] = (int)i;
		muggle_array_blocking_queue_put(&queue, &arr[i]);
	}
	for (muggle_atomic_int i = 0; i < capacity / 2; ++i)
	{
		int data = *(int*)muggle_array_blocking_queue_take(&queue);
		EXPECT_EQ(data, (int)i);
	}

	// push one and get one
	for (muggle_atomic_int i = 0; i < capacity * 5; ++i)
	{
		arr[i] = (int)i;
		muggle_array_blocking_queue_put(&queue, &arr[i]);

		int data = *(int*)muggle_array_blocking_queue_take(&queue);
		EXPECT_EQ(data, (int)i);
	}

	muggle_array_blocking_queue_destroy(&queue);
}

void producer_consumer(int cnt_producer, int cnt_consumer, int cnt_interval, int interval_ms,
	int capacity = 1024 * 2, int total = 10000)
{
	muggle_array_blocking_queue_t queue;
	int *arr = (int*)malloc(sizeof(int) * total);
	for (int i = 0; i < total; ++i)
	{
		arr[i] = i;
	}

	muggle_array_blocking_queue_init(&queue, capacity);

	muggle_atomic_int consumer_ready = 0;
	muggle_atomic_int total_read = 0;
	std::vector<std::thread> consumers;
	for (int i = 0; i < cnt_consumer; ++i)
	{
		consumers.push_back(std::thread([&]{
			muggle_atomic_fetch_add(&consumer_ready, 1, muggle_memory_order_relaxed);

			int recv_idx = 0;
			int cnt = 0;
			while (1)
			{
				void *data = muggle_array_blocking_queue_take(&queue);
				if (data == nullptr)
				{
					break;
				}

				if (cnt_producer == 1)
				{
					ASSERT_GE(*(int*)data, recv_idx);
				}

				if (*(int*)data != recv_idx)
				{
					// message loss
					recv_idx = *(int*)data;
				}
				++recv_idx;
				++cnt;
			}
			muggle_atomic_fetch_add(&total_read, cnt, muggle_memory_order_relaxed);
		}));
	}

	std::vector<std::thread> producers;
	muggle_atomic_int fetch_id = 0;
	for (int i = 0; i < cnt_producer; ++i)
	{
		producers.push_back(std::thread([&]{
			while (muggle_atomic_load(&consumer_ready, muggle_memory_order_acquire) != cnt_consumer);

			muggle_atomic_int idx;
			while (1)
			{
				idx = muggle_atomic_fetch_add(&fetch_id, 1, muggle_memory_order_relaxed);

				if (idx < total)
				{
					muggle_array_blocking_queue_put(&queue, &arr[idx]);
				}
				else
				{
					break;
				}

				if (cnt_interval != 0 && interval_ms != 0 && idx % cnt_interval == 0)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
				}
			}
		}));
	}

	for (auto &producer : producers)
	{
		producer.join();
	}

	for (int i = 0; i < cnt_consumer; ++i)
	{
		muggle_array_blocking_queue_put(&queue, nullptr);
	}

	for (auto &consumer : consumers)
	{
		consumer.join();
	}

	EXPECT_EQ(total_read, total);

	muggle_array_blocking_queue_destroy(&queue);
	free(arr);
}

TEST(array_blocking_queue, one_producer_one_consumer)
{
	producer_consumer(1, 1, g_cnt_interval, g_interval_ms);
}

TEST(array_blocking_queue, one_producer_mul_consumer)
{
	int hc = (int)std::thread::hardware_concurrency();
	hc /= 2;
	if (hc <= 1)
	{
		hc = 2;
	}

	producer_consumer(1, hc, g_cnt_interval, g_interval_ms);
}

TEST(array_blocking_queue, mul_producer_one_consumer)
{
	int hc = (int)std::thread::hardware_concurrency();
	hc /= 2;
	if (hc <= 1)
	{
		hc = 2;
	}

	producer_consumer(hc, 1, g_cnt_interval, g_interval_ms);
}

TEST(array_blocking_queue, mul_producer_mul_consumer)
{
	int hc = (int)std::thread::hardware_concurrency();
	hc /= 2;
	if (hc <= 1)
	{
		hc = 2;
	}

	producer_consumer(hc, hc, g_cnt_interval, g_interval_ms);
}
