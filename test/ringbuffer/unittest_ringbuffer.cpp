#include <thread>
#include <chrono>
#include <vector>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

int g_cnt_interval = 1024 * 2;
int g_interval_ms = 1;

int w_flags[] = {
	MUGGLE_RINGBUFFER_FLAG_WRITE_LOCK,
	MUGGLE_RINGBUFFER_FLAG_SINGLE_WRITER,
	MUGGLE_RINGBUFFER_FLAG_WRITE_BUSY_LOOP
};
int r_flags[] = {
	MUGGLE_RINGBUFFER_FLAG_READ_ALL | MUGGLE_RINGBUFFER_FLAG_READ_WAIT,
	MUGGLE_RINGBUFFER_FLAG_READ_ALL | MUGGLE_RINGBUFFER_FLAG_READ_BUSY_LOOP,
	MUGGLE_RINGBUFFER_FLAG_SINGLE_READER | MUGGLE_RINGBUFFER_FLAG_READ_BUSY_LOOP,
	MUGGLE_RINGBUFFER_FLAG_SINGLE_READER | MUGGLE_RINGBUFFER_FLAG_READ_WAIT,
	MUGGLE_RINGBUFFER_FLAG_MSG_READ_ONCE 
};
int invalid_r_flags[] = {
	MUGGLE_RINGBUFFER_FLAG_MSG_READ_ONCE | MUGGLE_RINGBUFFER_FLAG_READ_BUSY_LOOP
};

TEST(ringbuffer, usage_utils)
{
	muggle_atomic_int capacity = 1 << 10;
	muggle_atomic_int pos = 1 << ((sizeof(muggle_atomic_int) * 8) - 1);
	pos = ~pos;

	ASSERT_GT(pos, 0);
	ASSERT_LT(pos + 1, 0);

	muggle_atomic_int idx_in_ring = IDX_IN_POW_OF_2_RING(pos, capacity);
	ASSERT_GE(idx_in_ring, 0);
	ASSERT_LT(idx_in_ring, capacity);
	if (idx_in_ring == capacity - 1)
	{
		ASSERT_EQ(IDX_IN_POW_OF_2_RING(pos+1, capacity), 0);
	}
	else
	{
		ASSERT_EQ(IDX_IN_POW_OF_2_RING(pos+1, capacity), idx_in_ring + 1);
	}

	int k = 8;
	pos = pos - capacity * k - 1;
	muggle_atomic_int last_idx_in_ring = IDX_IN_POW_OF_2_RING(pos, capacity);
	++pos;
	for (muggle_atomic_int i = 0; i < capacity * k * 2; ++i)
	{
		idx_in_ring = IDX_IN_POW_OF_2_RING(pos, capacity);
		ASSERT_GE(idx_in_ring, 0);
		ASSERT_LT(idx_in_ring, capacity);
		if (last_idx_in_ring == capacity - 1)
		{
			ASSERT_EQ(idx_in_ring, 0);
		}
		else
		{
			ASSERT_EQ(idx_in_ring, last_idx_in_ring + 1);
		}
		last_idx_in_ring = idx_in_ring;
		++pos;
	}
}

void test_ringbuffer_init_destroy(int expect_ret, int flag)
{
	muggle_ringbuffer_t r;
	muggle_atomic_int capacity = 16;
	int ret;

	ret = muggle_ringbuffer_init(&r, capacity, flag);
	EXPECT_EQ(ret, expect_ret);
	if (expect_ret != MUGGLE_OK)
	{
		return;
	}

	EXPECT_EQ(r.capacity, capacity);

	ret = muggle_ringbuffer_destroy(&r);
	EXPECT_EQ(ret, 0);
}

TEST(ringbuffer, init_destroy)
{
	for (int w_flag = 0; w_flag < (int)(sizeof(w_flags) / sizeof(w_flags[0])); ++w_flag)
	{
		for (int r_flag = 0; r_flag < (int)(sizeof(r_flags) / sizeof(r_flags[0])); ++r_flag)
		{
			test_ringbuffer_init_destroy(MUGGLE_OK, w_flags[w_flag] | r_flags[r_flag]);
		}
	}

	for (int w_flag = 0; w_flag < (int)(sizeof(w_flags) / sizeof(w_flags[0])); ++w_flag)
	{
		for (int r_flag = 0; r_flag < (int)(sizeof(invalid_r_flags) / sizeof(invalid_r_flags[0])); ++r_flag)
		{
			test_ringbuffer_init_destroy(MUGGLE_ERR_INVALID_PARAM, w_flags[w_flag] | invalid_r_flags[r_flag]);
		}
	}
}

void test_write_read_in_single_thread(int flag)
{
	muggle_ringbuffer_t r;
	muggle_atomic_int capacity = 16;
	muggle_atomic_int pos = 0;
	int arr[160];

	muggle_ringbuffer_init(&r, capacity, flag);

	// push capacity / 2, and get all
	for (muggle_atomic_int i = 0; i < capacity / 2; ++i)
	{
		arr[i] = (int)i;
		muggle_ringbuffer_write(&r, &arr[i]);
	}
	for (muggle_atomic_int i = 0; i < capacity / 2; ++i)
	{
		int data = *(int*)muggle_ringbuffer_read(&r, pos++);
		EXPECT_EQ(data, (int)i);
	}

	// push one and get one
	for (muggle_atomic_int i = 0; i < capacity * 5; ++i)
	{
		arr[i] = (int)i;
		muggle_ringbuffer_write(&r, &arr[i]);

		int data = *(int*)muggle_ringbuffer_read(&r, pos++);
		EXPECT_EQ(data, (int)i);
	}

	muggle_ringbuffer_destroy(&r);
}
TEST(ringbuffer, write_read_in_single_thread)
{
	for (int w_flag = 0; w_flag < (int)(sizeof(w_flags) / sizeof(w_flags[0])); ++w_flag)
	{
		for (int r_flag = 0; r_flag < (int)(sizeof(r_flags) / sizeof(r_flags[0])); ++r_flag)
		{
			test_write_read_in_single_thread(w_flags[w_flag] | r_flags[r_flag]);
		}
	}
}

void producer_consumer(int flag, int cnt_producer, int cnt_consumer, int cnt_interval, int interval_ms)
{
	muggle_ringbuffer_t r;
	muggle_atomic_int capacity = 1024 * 16;
	muggle_atomic_int total = 1024 * 100;
	int *arr = (int*)malloc(sizeof(int) * total);
	for (int i = 0; i < total; ++i)
	{
		arr[i] = i;
	}
	muggle_atomic_int consumer_ready = 0;
	muggle_ringbuffer_init(&r, capacity, flag);

	std::vector<std::thread> consumers;
	for (int i = 0; i < cnt_consumer; ++i)
	{
		consumers.push_back(std::thread([&]{
			muggle_atomic_fetch_add(&consumer_ready, 1, muggle_memory_order_relaxed);

			muggle_atomic_int pos = 0;
			int recv_idx = 0;
			while (1)
			{
				void *data = muggle_ringbuffer_read(&r, pos++);
				if (data == nullptr)
				{
					break;
				}

				if (cnt_producer == 1)
				{
					if (flag & MUGGLE_RINGBUFFER_FLAG_MSG_READ_ONCE)
					{
						ASSERT_GE(*(int*)data, recv_idx);
					}
					else
					{
						EXPECT_EQ(*(int*)data, recv_idx);
					}
				}

				if (*(int*)data != recv_idx)
				{
					// message loss
					recv_idx = *(int*)data;
				}
				++recv_idx;
			}
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
				if (idx == total - 1)
				{
					if (flag & MUGGLE_RINGBUFFER_FLAG_MSG_READ_ONCE)
					{
						for (int i = 0; i < cnt_consumer; ++i)
						{
							muggle_ringbuffer_write(&r, nullptr);
						}
					}
					else
					{
						muggle_ringbuffer_write(&r, nullptr);
					}
					break;
				}

				if (idx < total)
				{
					muggle_ringbuffer_write(&r, &arr[idx]);
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
	for (auto &consumer : consumers)
	{
		consumer.join();
	}

	muggle_ringbuffer_destroy(&r);
	free(arr);
}

TEST(ringbuffer, one_producer_one_consumer)
{
	for (int w_flag = 0; w_flag < (int)(sizeof(w_flags) / sizeof(w_flags[0])); ++w_flag)
	{
		for (int r_flag = 0; r_flag < (int)(sizeof(r_flags) / sizeof(r_flags[0])); ++r_flag)
		{
			producer_consumer(w_flags[w_flag] | r_flags[r_flag], 1, 1, g_cnt_interval, g_interval_ms);
		}
	}
}
TEST(ringbuffer, one_producer_mul_consumer)
{
	int hc = (int)std::thread::hardware_concurrency();
	hc /= 2;
	if (hc <= 1)
	{
		hc = 2;
	}

	for (int w_flag = 0; w_flag < (int)(sizeof(w_flags) / sizeof(w_flags[0])); ++w_flag)
	{
		for (int r_flag = 0; r_flag < (int)(sizeof(r_flags) / sizeof(r_flags[0])); ++r_flag)
		{
			producer_consumer(w_flags[w_flag] | r_flags[r_flag], 1, hc, g_cnt_interval, g_interval_ms);
		}
	}
}

TEST(ringbuffer, mul_producer_one_consumer)
{
	int hc = (int)std::thread::hardware_concurrency();
	hc /= 2;
	if (hc <= 1)
	{
		hc = 2;
	}

	for (int w_flag = 0; w_flag < (int)(sizeof(w_flags) / sizeof(w_flags[0])); ++w_flag)
	{
		for (int r_flag = 0; r_flag < (int)(sizeof(r_flags) / sizeof(r_flags[0])); ++r_flag)
		{
			producer_consumer(w_flags[w_flag] | r_flags[r_flag], hc, 1, g_cnt_interval, g_interval_ms);
		}
	}
}

TEST(ringbuffer, mul_producer_mul_consumer)
{
	int hc = (int)std::thread::hardware_concurrency();
	hc /= 2;
	if (hc <= 1)
	{
		hc = 2;
	}

	for (int w_flag = 0; w_flag < (int)(sizeof(w_flags) / sizeof(w_flags[0])); ++w_flag)
	{
		for (int r_flag = 0; r_flag < (int)(sizeof(r_flags) / sizeof(r_flags[0])); ++r_flag)
		{
			producer_consumer(w_flags[w_flag] | r_flags[r_flag], hc, hc, g_cnt_interval, g_interval_ms);
		}
	}
}

