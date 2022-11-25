#include <vector>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

#if MUGGLE_C_HAVE_SYNC_OBJ

TEST(synclock, incr)
{
	int x = 0;

	muggle_sync_t synclock;
	muggle_synclock_init(&synclock);

	int cnt_thread = 8;
	int incr_per_thread = 1024;
	std::vector<std::thread> threads;
	for (int i = 0; i < cnt_thread; i++)
	{
		threads.push_back(std::thread([&] {
			for (int i = 0; i < incr_per_thread; i++)
			{
				muggle_synclock_lock(&synclock);
				x++;
				muggle_synclock_unlock(&synclock);
			}
		}));
	}

	for (auto &th : threads)
	{
		th.join();
	}

	EXPECT_EQ(x, cnt_thread * incr_per_thread);
}

TEST(synclock, order)
{
	int x = 0, y = 0;
	int x_data = 0, y_data = 0;

	muggle_sync_t synclock;
	muggle_synclock_init(&synclock);

	std::thread t1([&] {
		while (true)
		{
			muggle_synclock_lock(&synclock);
			if (x == 1)
			{
				muggle_synclock_unlock(&synclock);
				break;
			}
			muggle_synclock_unlock(&synclock);
			muggle_thread_yield();
		}
		EXPECT_EQ(x_data, 1);

		muggle_synclock_lock(&synclock);
		y_data = 1;
		y = 1;
		muggle_synclock_unlock(&synclock);
	});

	std::thread t2([&] {
		muggle_synclock_lock(&synclock);
		x_data = 1;
		x = 1;
		muggle_synclock_unlock(&synclock);

		while (true)
		{
			muggle_synclock_lock(&synclock);
			if (y == 1)
			{
				muggle_synclock_unlock(&synclock);
				break;
			}
			muggle_synclock_unlock(&synclock);
			muggle_thread_yield();
		}
		EXPECT_EQ(y_data, 1);
	});

	t1.join();
	t2.join();
}

#else // MUGGLE_C_HAVE_SYNC_OBJ

TEST(synclock, not_support)
{
}

#endif // MUGGLE_C_HAVE_SYNC_OBJ
