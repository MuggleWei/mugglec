#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

struct thread_test_arg {
	int ms;
};

muggle_thread_ret_t sleep_for_ms(void *arg)
{
	thread_test_arg *p = (thread_test_arg*)arg;
	std::this_thread::sleep_for(std::chrono::milliseconds(p->ms));

	return 0;
}

void assert_thread_not_equal(muggle_thread_id t1, muggle_thread_id t2)
{
	ASSERT_FALSE(muggle_thread_equal(t1, t2));
}

muggle_thread_ret_t detect_equal(void *arg)
{
	muggle_thread_id *tid = (muggle_thread_id*)arg;
	assert_thread_not_equal(*tid, muggle_thread_current_id());

	return 0;
}

TEST(thread, join)
{
	thread_test_arg arg;
	arg.ms = 50;

	muggle_thread_t thread;

	auto start = std::chrono::high_resolution_clock::now();
	muggle_thread_create(&thread, sleep_for_ms, &arg);
	muggle_thread_join(&thread);
	auto end = std::chrono::high_resolution_clock::now();

	auto diff = end - start;
	int elapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
	EXPECT_GE(elapsed, arg.ms);
}

TEST(thread, detach)
{
	thread_test_arg arg;
	arg.ms = 50;

	muggle_thread_t thread;

	auto start = std::chrono::high_resolution_clock::now();
	muggle_thread_create(&thread, sleep_for_ms, &arg);
	muggle_thread_detach(&thread);
	auto end = std::chrono::high_resolution_clock::now();

	auto diff = end - start;
	int elapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
	EXPECT_LT(elapsed, arg.ms);

	// for avoid of stack-use-after-free
	std::this_thread::sleep_for(std::chrono::milliseconds(arg.ms * 8));
}

TEST(thread, equal)
{
	muggle_thread_id t1 = muggle_thread_current_id();
	ASSERT_TRUE(muggle_thread_equal(t1, muggle_thread_current_id()));

	muggle_thread_t thread;
	muggle_thread_create(&thread, detect_equal, (void*)&t1);
	muggle_thread_join(&thread);

	std::thread th([&t1] {
		ASSERT_FALSE(muggle_thread_equal(t1, muggle_thread_current_id()));
	});
	th.join();
}
