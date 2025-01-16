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
	muggle_atomic_int capacity = (muggle_atomic_int)next_pow_of_2((uint64_t)5); // real capacity is 8, real allocate capacity = capacity - 1

	ASSERT_EQ(capacity, 8);

	muggle_ts_memory_pool_t pool;
	muggle_ts_memory_pool_init(&pool, capacity, sizeof(ts_data));

	ts_data *arr[8 - 1];
	for (int i = 0; i < capacity - 1; i++)
	{
		arr[i] = (ts_data*)muggle_ts_memory_pool_alloc(&pool);
		ASSERT_TRUE(arr[i] != NULL);
	}
	ASSERT_TRUE(muggle_ts_memory_pool_alloc(&pool) == NULL);

	muggle_ts_memory_pool_free(arr[0]);
	arr[0] = (ts_data*)muggle_ts_memory_pool_alloc(&pool);
	ASSERT_TRUE(arr[0] != NULL);

	for (int i = 0; i < capacity - 1; i++)
	{
		muggle_ts_memory_pool_free(arr[i]);
	}
	for (int i = 0; i < capacity - 1; i++)
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

	ASSERT_EQ(data_pos, (muggle_atomic_int)next_pow_of_2(capacity) - 1);
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

	ASSERT_EQ(IDX_IN_POW_OF_2_RING(pool.alloc_idx + 1, pool.capacity), pool.free_idx);

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

	ASSERT_EQ(pool.alloc_idx, pool.free_idx);

	muggle_ts_memory_pool_destroy(&pool);
	free(datas);
}

TEST(ts_memory_pool, busy_alloc_free)
{
	muggle_atomic_int capacity = 1024;
	int msg_cnt = (int)capacity * 512;
	ts_data **datas = (ts_data**)malloc(sizeof(ts_data*) * msg_cnt);
	for (int i = 0; i < msg_cnt; i++)
	{
		datas[i] = nullptr;
	}

	int hc = (int)std::thread::hardware_concurrency() * 2;
	if (hc <= 0)
	{
		hc = 4;
	}

	std::map<int, int> alloc_count;
	std::map<int, int> free_count;
	for (int i = 0; i < hc; i++)
	{
		alloc_count[i] = 0;
		free_count[i] = 0;
	}

	muggle_ts_memory_pool_t pool;
	muggle_ts_memory_pool_init(&pool, capacity, sizeof(ts_data));

	// run allocate threads
	std::vector<std::thread> alloc_threads;
	muggle_atomic_int alloc_idx = 0;
	for (int i = 0; i < hc; i++)
	{
		alloc_threads.push_back(std::thread([i, &pool, &datas, &alloc_count, &alloc_idx, &msg_cnt]{
			while (true)
			{
				muggle_atomic_int cur_idx = muggle_atomic_fetch_add(&alloc_idx, 1, muggle_memory_order_relaxed);
				if (cur_idx >= msg_cnt)
				{
					break;
				}

				ts_data *data = (ts_data*)muggle_ts_memory_pool_alloc(&pool);
				while (data == nullptr)
				{
					muggle_thread_yield();
					data = (ts_data*)muggle_ts_memory_pool_alloc(&pool);
				}

				datas[cur_idx] = data;

				alloc_count[i]++;
			}
		}));
	}

	// run free threads
	std::vector<std::thread> free_threads;
	muggle_atomic_int free_idx = 0;
	for (int i = 0; i < hc; i++)
	{
		free_threads.push_back(std::thread([i, &datas, &free_count, &free_idx, &msg_cnt]{
			while (true)
			{
				muggle_atomic_int cur_idx = muggle_atomic_fetch_add(&free_idx, 1, muggle_memory_order_relaxed);
				if (cur_idx >= msg_cnt)
				{
					break;
				}

				ts_data *data = datas[cur_idx];
				while (data == nullptr)
				{
					muggle_thread_yield();
					data = datas[cur_idx];
				}

				muggle_ts_memory_pool_free(data);

				free_count[i]++;
			}
		}));
	}

	// join threads
	for (int i = 0; i < hc; i++)
	{
		alloc_threads[i].join();
	}
	for (int i = 0; i < hc; i++)
	{
		free_threads[i].join();
	}

	// check message count
	int alloc_sum=0, free_sum=0;
	for (int i = 0; i < hc; i++)
	{
		// printf("alloc thread[%d]: %d\n", i, alloc_count[i]);
		alloc_sum += alloc_count[i];
	}
	for (int i = 0; i < hc; i++)
	{
		// printf("free thread[%d]: %d\n", i, free_count[i]);
		free_sum += free_count[i];
	}

	ASSERT_EQ(alloc_sum, msg_cnt);
	ASSERT_EQ(free_sum, msg_cnt);

	muggle_ts_memory_pool_destroy(&pool);
	free(datas);
}
