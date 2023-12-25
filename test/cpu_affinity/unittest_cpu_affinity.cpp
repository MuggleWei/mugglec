#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(cpu_affinity, cpu_mask_set_case1)
{
	muggle_cpu_mask_t mask;
	muggle_cpu_mask_zero(&mask);

	muggle_cpu_mask_set(&mask, 0);
	GTEST_ASSERT_TRUE(muggle_cpu_mask_isset(&mask, 0));
}

TEST(cpu_affinity, cpu_mask_set_case2)
{
	muggle_cpu_mask_t mask;
	muggle_cpu_mask_zero(&mask);

#if MUGGLE_PLATFORM_APPLE
	muggle_cpu_mask_set(&mask, 0);
	muggle_cpu_mask_set(&mask, 1);
	GTEST_ASSERT_FALSE(muggle_cpu_mask_isset(&mask, 0));
	GTEST_ASSERT_TRUE(muggle_cpu_mask_isset(&mask, 1));
#else
	muggle_cpu_mask_set(&mask, 0);
	muggle_cpu_mask_set(&mask, 1);
	GTEST_ASSERT_TRUE(muggle_cpu_mask_isset(&mask, 0));
	GTEST_ASSERT_TRUE(muggle_cpu_mask_isset(&mask, 1));
#endif
}

TEST(cpu_affinity, cpu_mask_clr_case1)
{
	muggle_cpu_mask_t mask;
	muggle_cpu_mask_zero(&mask);

	muggle_cpu_mask_set(&mask, 0);
	muggle_cpu_mask_set(&mask, 1);

	muggle_cpu_mask_clr(&mask, 0);
	GTEST_ASSERT_FALSE(muggle_cpu_mask_isset(&mask, 0));
	GTEST_ASSERT_TRUE(muggle_cpu_mask_isset(&mask, 1));
}
