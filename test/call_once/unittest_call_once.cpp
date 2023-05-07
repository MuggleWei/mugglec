#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

static int cnt = 0;
static void increase_cnt()
{
	cnt++;
}

TEST(call_once, mul_call_single_thread)
{
	cnt = 0;

	muggle_once_flag flag = MUGGLE_ONCE_FLAG_INIT;
	muggle_call_once(&flag, increase_cnt);
	muggle_call_once(&flag, increase_cnt);
	muggle_call_once(&flag, increase_cnt);
	muggle_call_once(&flag, increase_cnt);
	muggle_call_once(&flag, increase_cnt);

	EXPECT_EQ(cnt, 1);
}

static void increase_cnt_with_sleep()
{
	muggle_msleep(200);
	cnt++;
}

static muggle_thread_ret_t thread_routine(void *data)
{
	muggle_once_flag *flag = (muggle_once_flag *)data;
	muggle_call_once(flag, increase_cnt_with_sleep);

	return 0;
}

TEST(call_once, mul_call_mul_thread)
{
	cnt = 0;

	muggle_once_flag flag = MUGGLE_ONCE_FLAG_INIT;

	muggle_thread_t threads[16];
	for (int i = 0; i < 16; i++) {
		muggle_thread_create(&threads[i], thread_routine, (void *)&flag);
	}
	for (int i = 0; i < 16; i++) {
		muggle_thread_join(&threads[i]);
	}

	EXPECT_EQ(cnt, 1);
}
