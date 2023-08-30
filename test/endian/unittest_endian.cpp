#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(endian, compare_precompile_runtime)
{
	ASSERT_EQ(muggle_endianness(), MUGGLE_ENDIANNESS);
}

TEST(endian, check)
{
	int16_t i16 = 0x0001;
	char *p = (char*)&i16;
	char h = p[0];
	char l = p[1];

	if (muggle_endianness() == MUGGLE_LITTLE_ENDIAN)
	{
		ASSERT_EQ(h, 0x01);
		ASSERT_EQ(l, 0x00);
	}
	else
	{
		ASSERT_EQ(h, 0x00);
		ASSERT_EQ(l, 0x01);
	}
}

TEST(endian, swap_u16)
{
	uint16_t u16 = 0x0102;
	u16 = MUGGLE_ENDIAN_SWAP_16(u16);
	ASSERT_EQ(u16, 0x0201);

	u16 = 0xff00;
	u16 = MUGGLE_ENDIAN_SWAP_16(u16);
	ASSERT_EQ(u16, 0x00ff);

	u16 = 0x00ff;
	u16 = MUGGLE_ENDIAN_SWAP_16(u16);
	ASSERT_EQ(u16, 0xff00);
}

TEST(endian, swap_i16)
{
	int16_t i16 = 0x0102;
	i16 = MUGGLE_ENDIAN_SWAP_16(i16);
	ASSERT_EQ(i16, 0x0201);

	i16 = 0xff00;
	i16 = MUGGLE_ENDIAN_SWAP_16(i16);
	ASSERT_EQ(i16, 0x00ff);

	i16 = 0x00ff;
	i16 = MUGGLE_ENDIAN_SWAP_16(i16);
	uint16_t result = 0xff00;
	int16_t *i16_result = (int16_t*)&result;
	ASSERT_EQ(i16, *i16_result);
}

TEST(endian, swap_u32)
{
	uint32_t u32 = 0x01020304;
	u32 = MUGGLE_ENDIAN_SWAP_32(u32);
	ASSERT_EQ(u32, (uint32_t)0x04030201);

	u32 = 0xff000000;
	u32 = MUGGLE_ENDIAN_SWAP_32(u32);
	ASSERT_EQ(u32, (uint32_t)0x000000ff);

	u32 = 0x000000ff;
	u32 = MUGGLE_ENDIAN_SWAP_32(u32);
	ASSERT_EQ(u32, (uint32_t)0xff000000);
}

TEST(endian, swap_i32)
{
	int32_t i32 = 0x01020304;
	i32 = MUGGLE_ENDIAN_SWAP_32(i32);
	ASSERT_EQ(i32, 0x04030201);

	i32 = 0xff000000;
	i32 = MUGGLE_ENDIAN_SWAP_32(i32);
	ASSERT_EQ(i32, 0x000000ff);

	i32 = 0x000000ff;
	i32 = MUGGLE_ENDIAN_SWAP_32(i32);
	ASSERT_EQ(i32, (uint32_t)0xff000000);
}

TEST(endian, swap_u64)
{
	uint64_t u64 = 0x0102030405060708;
	u64 = MUGGLE_ENDIAN_SWAP_64(u64);
	ASSERT_EQ(u64, (uint64_t)0x0807060504030201);

	u64 = 0xff00000000000000;
	u64 = MUGGLE_ENDIAN_SWAP_64(u64);
	ASSERT_EQ(u64, (uint64_t)0x00000000000000ff);

	u64 = 0x00000000000000ff;
	u64 = MUGGLE_ENDIAN_SWAP_64(u64);
	ASSERT_EQ(u64, (uint64_t)0xff00000000000000);
}

TEST(endian, swap_i64)
{
	int64_t i64 = 0x0102030405060708;
	i64 = MUGGLE_ENDIAN_SWAP_64(i64);
	ASSERT_EQ(i64, 0x0807060504030201);

	i64 = 0xff00000000000000;
	i64 = MUGGLE_ENDIAN_SWAP_64(i64);
	ASSERT_EQ(i64, 0x00000000000000ff);

	i64 = 0x00000000000000ff;
	i64 = MUGGLE_ENDIAN_SWAP_64(i64);
	ASSERT_EQ(i64, (uint64_t)0xff00000000000000);
}
