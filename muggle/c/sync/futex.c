/******************************************************************************
 *  @file         futex.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec futex
 *****************************************************************************/

#include "futex.h"

#if MUGGLE_SUPPORT_FUTEX

#if MUGGLE_PLATFORM_WINDOWS

int muggle_futex_wait(muggle_atomic_int* futex_addr, muggle_atomic_int val, const struct timespec *timeout)
{
	BOOL ret;
	if (timeout == NULL)
	{
		ret = WaitOnAddress(futex_addr, &val, sizeof(val), INFINITE);
	}
	else
	{
		DWORD ms = timeout->tv_nsec / 1000000;
		if (ms == 0 && timeout->tv_nsec != 0)
		{
			ms = 1;
		}
		DWORD dwMilliseconds = (DWORD)(timeout->tv_sec * 1000 + ms);
		ret = WaitOnAddress(futex_addr, &val, sizeof(val), dwMilliseconds);
	}

	return ret ? 0 : 1;
}

void muggle_futex_wake_one(muggle_atomic_int* futex_addr)
{
	WakeByAddressSingle(futex_addr);
}

void muggle_futex_wake_all(muggle_atomic_int *futex_addr)
{
	WakeByAddressAll(futex_addr);
}

#else

#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <time.h>
#include <limits.h>

static int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
	return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
}

int muggle_futex_wait(muggle_atomic_int* futex_addr, muggle_atomic_int val, const struct timespec *timeout)
{
	return futex(futex_addr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, val, timeout, NULL, 0);
}

void muggle_futex_wake_one(muggle_atomic_int* futex_addr)
{
	futex(futex_addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0);
}

void muggle_futex_wake_all(muggle_atomic_int *futex_addr)
{
	futex(futex_addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, NULL, 0);
}

#endif  // MUGGLE_PLATFORM_WINDOWS

#endif  // MUGGLE_SUPPORT_FUTEX
