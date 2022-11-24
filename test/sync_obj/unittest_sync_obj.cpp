#include <vector>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

#if MUGGLE_C_HAVE_SYNC_OBJ

TEST(sync_obj, wait_ret_immediately)
{
	muggle_sync_t val = 0;
	muggle_sync_wait(&val, 1, NULL);
}

TEST(sync_obj, wake_one_and_memory_order)
{
	muggle_sync_t x = 0, y = 0;
	int x_data = 0, y_data = 0;

	std::thread t1([&]{
		muggle_sync_wait(&x, 0, NULL);
		EXPECT_EQ(x, 1);
		EXPECT_EQ(x_data, 1);

		y_data = 1;
		y = 1;
		muggle_sync_wake_one(&y);
	});
	std::thread t2([&]{
		x_data = 1;
		x = 1;
		muggle_sync_wake_one(&x);

		muggle_sync_wait(&y, 0, NULL);
		EXPECT_EQ(y, 1);
		EXPECT_EQ(y_data, 1);
	});
	t1.join();
	t2.join();
}

TEST(sync_obj, wake_all)
{
	muggle_atomic_int x = 0;
	int waiter_num = 10;
	muggle_sync_t y = 0;
	std::vector<std::thread> waiters;

	for (int i = 0; i < waiter_num; ++i)
	{
		waiters.push_back(std::thread([&]{
			muggle_atomic_fetch_add(&x, 1, muggle_memory_order_relaxed);
			muggle_sync_wait(&y, 0, NULL);
		}));
	}

	std::thread wakeup([&]{
		while (
			muggle_atomic_load(&x, muggle_memory_order_relaxed) != waiter_num)
		{
			muggle_thread_yield();
		}
		muggle_atomic_store(&y, 1, muggle_memory_order_relaxed);
		muggle_sync_wake_all(&y);
	});

	for (auto &waiter : waiters)
	{
		waiter.join();
	}
	wakeup.join();

	EXPECT_EQ(x, waiter_num);
	EXPECT_EQ(y, 1);
}

#else

TEST(sync_obj, not_support)
{
}

#endif
