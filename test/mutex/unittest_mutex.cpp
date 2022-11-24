#include <vector>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(mutex, incr)
{
	int x = 0;

	muggle_mutex_t mtx;
	muggle_mutex_init(&mtx);

	int cnt_thread = 8;
	int incr_per_thread = 1024;
	std::vector<std::thread> threads;
	for (int i = 0; i < cnt_thread; i++)
	{
		threads.push_back(std::thread([&] {
			for (int i = 0; i < incr_per_thread; i++)
			{
				muggle_mutex_lock(&mtx);
				x++;
				muggle_mutex_unlock(&mtx);
			}
		}));
	}

	for (auto &th : threads)
	{
		th.join();
	}

	muggle_mutex_destroy(&mtx);

	EXPECT_EQ(x, cnt_thread * incr_per_thread);
}

TEST(mutex, order)
{
	int x = 0, y = 0;
	int x_data = 0, y_data = 0;

	muggle_mutex_t mtx;
	muggle_mutex_init(&mtx);

	muggle_condition_variable_t cv;
	muggle_condition_variable_init(&cv);

	std::thread t1([&] {
		muggle_mutex_lock(&mtx);

		while (x != 1)
		{
			muggle_condition_variable_wait(&cv, &mtx, NULL);
		}
		EXPECT_EQ(x_data, 1);

		y_data = 1;
		y = 1;

		muggle_mutex_unlock(&mtx);
		muggle_condition_variable_notify_one(&cv);
	});

	std::thread t2([&] {
		muggle_mutex_lock(&mtx);

		x_data = 1;
		x = 1;

		muggle_mutex_unlock(&mtx);
		muggle_condition_variable_notify_one(&cv);

		muggle_mutex_lock(&mtx);

		while (y != 1)
		{
			muggle_condition_variable_wait(&cv, &mtx, NULL);
		}
		EXPECT_EQ(y_data, 1);

		muggle_mutex_unlock(&mtx);
	});

	t1.join();
	t2.join();

	muggle_condition_variable_destroy(&cv);
	muggle_mutex_destroy(&mtx);
}
