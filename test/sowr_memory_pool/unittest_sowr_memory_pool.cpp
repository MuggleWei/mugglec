#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

struct sowr_data
{
	int info;
	int idx;
	int is_end;
};

TEST(sowr_memory_pool, basic)
{
	muggle_atomic_int capacity = 8;

	muggle_sowr_memory_pool_t pool;
	muggle_sowr_memory_pool_init(&pool, capacity, sizeof(sowr_data));
	
	sowr_data *arr[8];

	for (int i = 0; i < capacity - 1; ++i)
	{
		sowr_data *p = (sowr_data*)muggle_sowr_memory_pool_alloc(&pool);
		ASSERT_TRUE(p != NULL);
		muggle_sowr_block_head_t *head = (muggle_sowr_block_head_t*)p - 1;
		ASSERT_EQ(head->pool, &pool);
		EXPECT_EQ(head->block_idx, i);
		arr[i] = p;
	}
	ASSERT_TRUE(muggle_sowr_memory_pool_alloc(&pool) == NULL);

	muggle_sowr_memory_pool_free(arr[0]);
	sowr_data *p = (sowr_data*)muggle_sowr_memory_pool_alloc(&pool);
	muggle_sowr_block_head_t *head = (muggle_sowr_block_head_t*)p - 1;
	ASSERT_EQ(head->pool, &pool);
	EXPECT_EQ(head->block_idx, 7);
	arr[7] = p;

	// free all one by one
	ASSERT_FALSE(muggle_sowr_memory_pool_is_all_free(&pool));
	for (int i = 1; i < capacity; ++i)
	{
		muggle_sowr_memory_pool_free(arr[i]);
	}
	ASSERT_TRUE(muggle_sowr_memory_pool_is_all_free(&pool));

	// free last
	for (int i = 0; i < capacity - 1; ++i)
	{
		p = (sowr_data*)muggle_sowr_memory_pool_alloc(&pool);
	}
	ASSERT_FALSE(muggle_sowr_memory_pool_is_all_free(&pool));
	muggle_sowr_memory_pool_free(p);
	ASSERT_TRUE(muggle_sowr_memory_pool_is_all_free(&pool));

	muggle_sowr_memory_pool_destroy(&pool);
}

static thread_local int have_pool_flag = 0;
static thread_local muggle_sowr_memory_pool_t *pool = nullptr;
TEST(sowr_memory_pool, mul_pool_alloc_free)
{
	std::mutex mutex;
	std::queue<sowr_data*> data_queue;
	int capacity = 1024 * 16;
	int cnt = 1024 * 100;

	auto fn_producer = [&](int info){
		if (have_pool_flag == 0)
		{
			pool = (muggle_sowr_memory_pool_t*)malloc(sizeof(muggle_sowr_memory_pool_t));
			have_pool_flag = 1;
		}

		muggle_sowr_memory_pool_init(pool, capacity, sizeof(sowr_data));
		for (int i = 0; i < cnt; ++i)
		{
			sowr_data *data = (sowr_data*)muggle_sowr_memory_pool_alloc(pool);
			if (!data)
			{
				continue;
			}
			data->info = info;
			data->idx = i;
			data->is_end = 0;

			if (info == 0 && (i % 2 == 0)) // simulate message loss
			{
				continue;
			}

			{
				std::unique_lock<std::mutex> lock(mutex);
				data_queue.push(data);
			}
		}

		sowr_data *data = (sowr_data*)muggle_sowr_memory_pool_alloc(pool);
		while (!data)
		{
			data = (sowr_data*)muggle_sowr_memory_pool_alloc(pool);
		}
		data->info = info;
		data->idx = 0;
		data->is_end = 1;

		{
			std::unique_lock<std::mutex> lock(mutex);
			data_queue.push(data);
		}

		// wait all free
		while (!muggle_sowr_memory_pool_is_all_free(pool));
		muggle_sowr_memory_pool_destroy(pool);

		free(pool);
	};


	std::thread consumer([&]{
		int last_idxs[2] = {-1, -1};
		int is_end[2] = {0, 0};
		while (1)
		{
			sowr_data *p = nullptr;
			{
				std::unique_lock<std::mutex> lock(mutex);
				if (data_queue.empty())
				{
					continue;
				}
				p = data_queue.front();
				data_queue.pop();
			}

			ASSERT_TRUE(p != NULL);
			if (!p->is_end)
			{
				ASSERT_GT(p->idx, last_idxs[p->info]);
				last_idxs[p->info] = p->idx;
			}

			muggle_sowr_memory_pool_free(p);

			if (p->is_end)
			{
				is_end[p->info] = 1;
				if (is_end[0] == 1 && is_end[1] == 1)
				{
					break;
				}
			}
		}
	});

	std::thread loss_producer(fn_producer, 0);
	std::thread normal_producer(fn_producer, 1);

	consumer.join();
	loss_producer.join();
	normal_producer.join();
}
