#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(crypt_utils, rotate_8x8bits)
{
	muggle_64bit_block_t block;

	memset(&block, 0, sizeof(block));
	block.bytes[0] = 0x01;
	muggle_8x8bit_rotate_row(&block, 0, 1);
	ASSERT_EQ(block.bytes[0], 0x02);

	memset(&block, 0, sizeof(block));
	block.bytes[0] = 0x01;
	muggle_8x8bit_rotate_col(&block, 0, 1);
	ASSERT_EQ(block.bytes[0], 0x00);
	ASSERT_EQ(block.bytes[1], 0x01);
}
