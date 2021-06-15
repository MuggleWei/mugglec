/******************************************************************************
 *  @file         atomic.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec atomic source file
 *****************************************************************************/

#include "atomic.h"

#if MUGGLE_PLATFORM_WINDOWS

int muggle_win_atomic_cmp_exch32(muggle_atomic_int32 *dst, muggle_atomic_int32 *expected, muggle_atomic_int32 desired)
{
	muggle_atomic_int32 ret = InterlockedCompareExchange(dst, desired, *expected);
	if (ret == *expected)
	{
		return 1;
	}
	*expected = ret;
	return 0;
}

int muggle_win_atomic_cmp_exch64(muggle_atomic_int64 *dst, muggle_atomic_int64 *expected, muggle_atomic_int64 desired)
{
	muggle_atomic_int64 ret = InterlockedCompareExchange64(dst, desired, *expected);
	if (ret == *expected)
	{
		return 1;
	}
	*expected = ret;
	return 0;
}

#endif
