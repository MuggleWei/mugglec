#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(futex, wait_ret_immediately)
{
	muggle_atomic_int val = 0;
	muggle_futex_wait(&val, 1);
}

TEST(futex, wait_and_wake)
{
	muggle_atomic_int x = 0, y = 0;

	std::thread t1([&]{
		muggle_futex_wait(&x, 0);
		muggle_atomic_store(&y, 1, muggle_memory_order_relaxed);
		muggle_futex_wake(&y);
	});
	std::thread t2([&]{
		muggle_atomic_store(&x, 1, muggle_memory_order_relaxed);
		muggle_futex_wake(&x);
		muggle_futex_wait(&y, 0);
	});
	t1.join();
	t2.join();

	EXPECT_EQ(x, 1);
	EXPECT_EQ(y, 1);
}
