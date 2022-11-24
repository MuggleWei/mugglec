#include <vector>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(spinlock, incr)
{
	int x = 0;

	muggle_spinlock_t spinlock;
	muggle_spinlock_init(&spinlock);

	int cnt_thread = 8;
	int incr_per_thread = 1024;
	std::vector<std::thread> threads;
	for (int i = 0; i < cnt_thread; i++)
	{
		threads.push_back(std::thread([&] {
			for (int i = 0; i < incr_per_thread; i++)
			{
				muggle_spinlock_lock(&spinlock);
				x++;
				muggle_spinlock_unlock(&spinlock);
			}
		}));
	}

	for (auto &th : threads)
	{
		th.join();
	}

	EXPECT_EQ(x, cnt_thread * incr_per_thread);
}

TEST(spinlock, order)
{
	int x = 0, y = 0;
	int x_data = 0, y_data = 0;

	muggle_spinlock_t spinlock;
	muggle_spinlock_init(&spinlock);

	std::thread t1([&] {
		while (true)
		{
			muggle_spinlock_lock(&spinlock);
			if (x == 1)
			{
				muggle_spinlock_unlock(&spinlock);
				break;
			}
			muggle_spinlock_unlock(&spinlock);
			muggle_thread_yield();
		}
		EXPECT_EQ(x_data, 1);

		muggle_spinlock_lock(&spinlock);
		y_data = 1;
		y = 1;
		muggle_spinlock_unlock(&spinlock);
	});

	std::thread t2([&] {
		muggle_spinlock_lock(&spinlock);
		x_data = 1;
		x = 1;
		muggle_spinlock_unlock(&spinlock);

		while (true)
		{
			muggle_spinlock_lock(&spinlock);
			if (y == 1)
			{
				muggle_spinlock_unlock(&spinlock);
				break;
			}
			muggle_spinlock_unlock(&spinlock);
			muggle_thread_yield();
		}
		EXPECT_EQ(y_data, 1);
	});

	t1.join();
	t2.join();
}
