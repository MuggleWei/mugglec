#include <vector>
#include <map>
#include <thread>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

struct ts_data
{
	int idx;
	int thread_idx;
};

TEST(ts_memory_pool, single_thread)
{
	muggle_atomic_int capacity = next_pow_of_2(5); // real capacity is 8, real allocate capacity = capacity - 1
	muggle_atomic_int real_capacity = capacity - 1;

	ASSERT_EQ(capacity, 8);
	ASSERT_EQ(real_capacity, 7);

	muggle_ts_memory_pool_t pool;
	muggle_ts_memory_pool_init(&pool, capacity, sizeof(ts_data));

	ts_data *arr[8];
	for (int i = 0; i < real_capacity; i++)
	{
		arr[i] = (ts_data*)muggle_ts_memory_pool_alloc(&pool);
		ASSERT_TRUE(arr[i] != NULL);
	}
	ASSERT_TRUE(muggle_ts_memory_pool_alloc(&pool) == NULL);

	muggle_ts_memory_pool_free(arr[0]);
	arr[0] = (ts_data*)muggle_ts_memory_pool_alloc(&pool);
	ASSERT_TRUE(arr[0] != NULL);

	for (int i = 0; i < real_capacity; i++)
	{
		muggle_ts_memory_pool_free(arr[i]);
	}
	for (int i = 0; i < real_capacity; i++)
	{
		arr[i] = (ts_data*)muggle_ts_memory_pool_alloc(&pool);
		ASSERT_TRUE(arr[i] != NULL);
	}
	ASSERT_TRUE(muggle_ts_memory_pool_alloc(&pool) == NULL);

	muggle_ts_memory_pool_destroy(&pool);
}

TEST(ts_memory_pool, mul_thread)
{
	muggle_atomic_int capacity = 1024 * 512;
	muggle_ts_memory_pool_t pool;
	muggle_ts_memory_pool_init(&pool, capacity, sizeof(ts_data));

	int hc = (int)std::thread::hardware_concurrency() * 2;
	if (hc <= 0)
	{
		hc = 4;
	}
	bool ready = false;
	std::vector<std::thread> threads;
	ts_data **datas = (ts_data**)malloc(sizeof(ts_data*) * next_pow_of_2(capacity));
	muggle_atomic_int data_pos = 0;

	// allocate
	for (int i = 0; i < hc; i++)
	{
		int thread_idx = i;
		threads.push_back(std::thread([&, thread_idx]{
			while (!ready)
			{
				std::this_thread::yield();
			}

			ts_data *p = nullptr;
			while ((p = (ts_data*)muggle_ts_memory_pool_alloc(&pool)) != nullptr)
			{
				muggle_atomic_int pos = muggle_atomic_fetch_add(&data_pos, 1, muggle_memory_order_relaxed);
				p->idx = (int)pos;
				p->thread_idx = thread_idx;
				datas[pos] = p;
			}
		}));
	}

	ready = true;

	for (int i = 0; i < hc; i++)
	{
		threads[i].join();
	}
	threads.clear();

	ASSERT_EQ(data_pos, next_pow_of_2(capacity) - 1);
	ASSERT_EQ(muggle_ts_memory_pool_alloc(&pool), nullptr);

	std::map<int, int> thread_count;
	for (int i = 0; i < hc; i++)
	{
		thread_count[i] = 0;
	}
	for (int i = 0; i < (int)data_pos; i++)
	{
		ASSERT_TRUE(thread_count.find(datas[i]->thread_idx) != thread_count.end());
		thread_count[datas[i]->thread_idx]++;
	}
//	for (int i = 0; i < hc; i++)
//	{
//		printf("%d - %d\n", i, thread_count[i]);
//	}

	ASSERT_EQ(pool.alloc_cursor, pool.free_cursor);

	// free
	ready = false;
	muggle_atomic_int data_idx = 0;
	for (int i = 0; i < hc; i++)
	{
		threads.push_back(std::thread([&]{
			while (!ready)
			{
				std::this_thread::yield();
			}

			while (true)
			{
				muggle_atomic_int pos = muggle_atomic_fetch_add(&data_idx, 1, muggle_memory_order_relaxed);
				if (pos >= data_pos)
				{
					break;
				}

				muggle_ts_memory_pool_free(datas[pos]);
			}
		}));
	}

	ready = true;

	for (int i = 0; i < hc; i++)
	{
		threads[i].join();
	}
	threads.clear();

	ASSERT_EQ(pool.alloc_cursor + next_pow_of_2(capacity) - 1, pool.free_cursor);

	muggle_ts_memory_pool_destroy(&pool);
	free(datas);
}
