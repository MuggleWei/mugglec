#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

uint32_t po2_arr[] = {
	0,
	0x01 << 0,
	0x01 << 1,
	0x01 << 2,
	0x01 << 3,
	0x01 << 4,
	0x01 << 5,
	0x01 << 6,
	0x01 << 7,
	0x01 << 8,
	0x01 << 9,
	0x01 << 10,
	0x01 << 11,
	0x01 << 12,
	0x01 << 13,
	0x01 << 14,
	0x01 << 15,
	0x01 << 16,
	0x01 << 17,
	0x01 << 18,
	0x01 << 19,
	0x01 << 20,
	0x01 << 21,
	0x01 << 22,
	0x01 << 23,
	0x01 << 24,
	0x01 << 25,
	0x01 << 26,
	0x01 << 27,
	0x01 << 28,
	0x01 << 29,
	0x01 << 30,
};

TEST(baseutils, is_pow_of_2)
{
	EXPECT_TRUE(MUGGLE_IS_POW_OF_2(2));
	EXPECT_FALSE(MUGGLE_IS_POW_OF_2(3));
	EXPECT_TRUE(MUGGLE_IS_POW_OF_2(4));
	EXPECT_FALSE(MUGGLE_IS_POW_OF_2(5));
	EXPECT_FALSE(MUGGLE_IS_POW_OF_2(6));
	EXPECT_FALSE(MUGGLE_IS_POW_OF_2(7));
	EXPECT_TRUE(MUGGLE_IS_POW_OF_2(8));

	for (size_t i = 0; i < sizeof(po2_arr) / sizeof(po2_arr[0]); ++i)
	{
		EXPECT_TRUE(MUGGLE_IS_POW_OF_2(po2_arr[i]));
		// for save time, only test i < 16
		if (i < 16)
		{
			for (uint32_t j = po2_arr[i]+1; j < po2_arr[i+1]; ++j)
			{
				EXPECT_FALSE(MUGGLE_IS_POW_OF_2(j));
			}
		}
	}
}

TEST(baseutils, round_pow_of_2)
{
	EXPECT_EQ(MUGGLE_ROUND_UP_POW_OF_2_MUL(3, 2), 4);
	EXPECT_EQ(MUGGLE_ROUND_UP_POW_OF_2_MUL(3, 4), 4);
	EXPECT_EQ(MUGGLE_ROUND_UP_POW_OF_2_MUL(3, 8), 8);

	EXPECT_EQ(MUGGLE_ROUND_UP_POW_OF_2_MUL(9, 2), 10);
	EXPECT_EQ(MUGGLE_ROUND_UP_POW_OF_2_MUL(9, 4), 12);
	EXPECT_EQ(MUGGLE_ROUND_UP_POW_OF_2_MUL(9, 8), 16);

	// for save time, only test i < 16
	for (uint32_t i = 3; i < 0x01 << 16; ++i)
	{
		for (uint32_t j = 1; j < 30; ++j)
		{
			uint32_t x = MUGGLE_ROUND_UP_POW_OF_2_MUL(i,0x01<<j);
			EXPECT_EQ(x % 0x01<<j, (uint32_t)0);
			EXPECT_LT(x-i, (uint32_t)0x01<<j);
		}
	}
}

TEST(baseutils, next_pow_of_2)
{
	EXPECT_EQ(muggle_next_pow_of_2((uint64_t)2), (uint64_t)2);
	EXPECT_EQ(muggle_next_pow_of_2((uint64_t)3), (uint64_t)4);
	EXPECT_EQ(muggle_next_pow_of_2((uint64_t)4), (uint64_t)4);
	EXPECT_EQ(muggle_next_pow_of_2((uint64_t)5), (uint64_t)8);
	EXPECT_EQ(muggle_next_pow_of_2((uint64_t)6), (uint64_t)8);
	EXPECT_EQ(muggle_next_pow_of_2((uint64_t)7), (uint64_t)8);
	EXPECT_EQ(muggle_next_pow_of_2((uint64_t)8), (uint64_t)8);

	// for save time, only test i < 16
	for (uint32_t i = 1; i < 16; ++i)
	{
		uint32_t lower = (0x01 << i) + 1u;
		uint32_t upper = 0x01 << (i + 1);
		for (uint32_t j = lower; j <= upper; ++j)
		{
			uint32_t x = (uint32_t)muggle_next_pow_of_2((uint64_t)j);
			EXPECT_EQ(x, upper);
		}
	}
}
