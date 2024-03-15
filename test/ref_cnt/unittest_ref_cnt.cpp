#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(ref_cnt, retain_release)
{
	muggle_ref_cnt_t ref;
	muggle_ref_cnt_init(&ref, 1);

	int n = 0;
	
	n = muggle_ref_cnt_retain(&ref);
	ASSERT_EQ(n, 2);
	ASSERT_EQ(n, muggle_ref_cnt_load(&ref, muggle_memory_order_relaxed));
	ASSERT_EQ(n, muggle_ref_cnt_val(&ref));

	n = muggle_ref_cnt_release(&ref);
	ASSERT_EQ(n, 1);
	ASSERT_EQ(n, muggle_ref_cnt_load(&ref, muggle_memory_order_relaxed));
	ASSERT_EQ(n, muggle_ref_cnt_val(&ref));

	n = muggle_ref_cnt_release(&ref);
	ASSERT_EQ(n, 0);
	ASSERT_EQ(n, muggle_ref_cnt_load(&ref, muggle_memory_order_relaxed));
	ASSERT_EQ(n, muggle_ref_cnt_val(&ref));

	n = muggle_ref_cnt_release(&ref);
	ASSERT_EQ(n, -1);
	ASSERT_EQ(muggle_ref_cnt_val(&ref), 0);

	n = muggle_ref_cnt_retain(&ref);
	ASSERT_EQ(n, -1);
	ASSERT_EQ(muggle_ref_cnt_val(&ref), 0);
}
