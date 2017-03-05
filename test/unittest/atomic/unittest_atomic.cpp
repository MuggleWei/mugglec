#include "gtest/gtest.h"
#include "muggle/base_c/base_c.h"

TEST(Atomic, Size)
{
	volatile int16_t i16;
#if MUGGLE_PLATFORM_WINDOWS
	volatile long ilong;
#else
	volatile int32_t i32;
#endif
	volatile int64_t i64;

	i16 = 0;
#if MUGGLE_PLATFORM_WINDOWS
	ilong = 0;
#else
	i32 = 0;
#endif
	i64 = 0;

	ASSERT_EQ((int)sizeof(i16), 2);
#if MUGGLE_PLATFORM_WINDOWS
	ASSERT_EQ((int)sizeof(ilong), 4);
#else
	ASSERT_EQ((int)sizeof(i32), 4);
#endif
	ASSERT_EQ((int)sizeof(i64), 8);
}

TEST(Atomic, Set)
{
	char ch;
	volatile int16_t i16;
#if MUGGLE_PLATFORM_WINDOWS
	volatile long ilong;
#else
	volatile int32_t i32;
#endif
	volatile int64_t i64;
	void *volatile p, *volatile q;

	ch = '\0';
	i16 = 0;
#if MUGGLE_PLATFORM_WINDOWS
	ilong = 0;
#else
	i32 = 0;
#endif
	i64 = 0;
	p = NULL;
	q = (void*)&ch;

	ATOMIC_Set_16(i16, 1);
#if MUGGLE_PLATFORM_WINDOWS
	ATOMIC_Set_32(ilong, 1);
#else
	ATOMIC_Set_32(i32, 1);
#endif
	ATOMIC_Set_64(i64, 1);
	ATOMIC_Set_Pointer(p, q);

	
	EXPECT_EQ(i16, 1);
#if MUGGLE_PLATFORM_WINDOWS
	EXPECT_EQ(ilong, 1);
#else
	EXPECT_EQ(i32, 1);
#endif
	EXPECT_EQ(i64, 1);
	EXPECT_EQ((void*)p, (void*)q);
	EXPECT_EQ(*(char*)p, ch);
}

TEST(Atomic, InDecrement)
{
	volatile int16_t i16;
#if MUGGLE_PLATFORM_WINDOWS
	volatile long ilong;
#else
	volatile int32_t i32;
#endif
	volatile int64_t i64;

	i16 = 0;
#if MUGGLE_PLATFORM_WINDOWS
	ilong = 0;
#else
	i32 = 0;
#endif
	i64 = 0;

	ATOMIC_Increment_16(i16);
#if MUGGLE_PLATFORM_WINDOWS
	ATOMIC_Increment_32(ilong);
#else
	ATOMIC_Increment_32(i32);
#endif
	ATOMIC_Increment_64(i64);

	EXPECT_EQ(i16, 1);
#if MUGGLE_PLATFORM_WINDOWS
	EXPECT_EQ(ilong, 1);
#else
	EXPECT_EQ(i32, 1);
#endif
	EXPECT_EQ(i64, 1);

	ATOMIC_Decrement_16(i16);
#if MUGGLE_PLATFORM_WINDOWS
	ATOMIC_Decrement_32(ilong);
#else
	ATOMIC_Decrement_32(i32);
#endif
	ATOMIC_Decrement_64(i64);

	EXPECT_EQ(i16, 0);
#if MUGGLE_PLATFORM_WINDOWS
	EXPECT_EQ(ilong, 0);
#else
	EXPECT_EQ(i32, 0);
#endif
	EXPECT_EQ(i64, 0);
}

TEST(Atomic, AddSub)
{
#if MUGGLE_PLATFORM_WINDOWS
	volatile long ilong;
#else
	volatile int32_t i32;
#endif
	volatile int64_t i64;

#if MUGGLE_PLATFORM_WINDOWS
	ilong = 0;
#else
	i32 = 0;
#endif
	i64 = 0;

#if MUGGLE_PLATFORM_WINDOWS
	ATOMIC_Add_32(ilong, 5);
#else
	ATOMIC_Add_32(i32, 5);
#endif
	ATOMIC_Add_64(i64, 5);

#if MUGGLE_PLATFORM_WINDOWS
	EXPECT_EQ(ilong, 5);
#else
	EXPECT_EQ(i32, 5);
#endif
	EXPECT_EQ(i64, 5);
}

TEST(Atomic, CAS)
{
	char ch = '\0';
	volatile int16_t i16;
#if MUGGLE_PLATFORM_WINDOWS
	volatile long ilong;
#else
	volatile int32_t i32;
#endif
	volatile int64_t i64;
	void *volatile p, *volatile q;

	ch = '\0';
	i16 = 0;
#if MUGGLE_PLATFORM_WINDOWS
	ilong = 0;
#else
	i32 = 0;
#endif
	i64 = 0;
	p = NULL;
	q = (void*)&ch;

	ATOMIC_CAS_16(i16, 0, 5);
#if MUGGLE_PLATFORM_WINDOWS
	ATOMIC_CAS_32(ilong, 0, 5);
#else
	ATOMIC_CAS_32(i32, 0, 5);
#endif
	ATOMIC_CAS_64(i64, 0, 5);
	ATOMIC_CAS_Pointer(p, NULL, q);

	EXPECT_EQ(i16, 5);
#if MUGGLE_PLATFORM_WINDOWS
	EXPECT_EQ(ilong, 5);
#else
	EXPECT_EQ(i32, 5);
#endif
	EXPECT_EQ(i64, 5);
	EXPECT_EQ((void*)p, (void*)q);
	EXPECT_EQ(*(char*)p, ch);
}