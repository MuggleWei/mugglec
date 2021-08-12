#include <vector>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

#if MUGGLE_SUPPORT_FUTEX

TEST(futex, wait_ret_immediately)
{
	muggle_atomic_int val = 0;
	muggle_futex_wait(&val, 1, NULL);
}

TEST(futex, wait_and_wake_one)
{
	muggle_atomic_int x = 0, y = 0;

	std::thread t1([&]{
		muggle_futex_wait(&x, 0, NULL);
		muggle_atomic_store(&y, 1, muggle_memory_order_relaxed);
		muggle_futex_wake_one(&y);
	});
	std::thread t2([&]{
		muggle_atomic_store(&x, 1, muggle_memory_order_relaxed);
		muggle_futex_wake_one(&x);
		muggle_futex_wait(&y, 0, NULL);
	});
	t1.join();
	t2.join();

	EXPECT_EQ(x, 1);
	EXPECT_EQ(y, 1);
}

TEST(futex, wait_and_wake_all)
{
	muggle_atomic_int x = 0, y = 0;
	std::vector<std::thread> waiters;
	int waiter_num = 10;

	for (int i = 0; i < waiter_num; ++i)
	{
		waiters.push_back(std::thread([&]{
			muggle_atomic_fetch_add(&x, 1, muggle_memory_order_relaxed);
			muggle_futex_wait(&y, 0, NULL);
		}));
	}

	std::thread wakeup([&]{
		while (muggle_atomic_load(&x, muggle_memory_order_relaxed) != waiter_num);
		muggle_atomic_store(&y, 1, muggle_memory_order_relaxed);
		muggle_futex_wake_all(&y);
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

TEST(futex, not_support)
{
}

#endif /* #if MUGGLE_SUPPORT_FUTEX */
