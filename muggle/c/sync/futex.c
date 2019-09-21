/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "futex.h"


#if MUGGLE_PLATFORM_WINDOWS

void muggle_futex_wait(muggle_atomic_int* futex_addr, muggle_atomic_int val, const struct timespec *timeout)
{
	if (timeout == NULL)
	{
		WaitOnAddress(futex_addr, &val, sizeof(val), INFINITE);
	}
	else
	{
		DWORD ms = timeout->tv_nsec / 1000000;
		if (ms == 0 && timeout->tv_nsec != 0)
		{
			ms = 1;
		}
		DWORD dwMilliseconds = timeout->tv_sec * 1000 + ms;
		WaitOnAddress(futex_addr, &val, sizeof(val), dwMilliseconds);
	}
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

void muggle_futex_wait(muggle_atomic_int* futex_addr, muggle_atomic_int val, const struct timespec *timeout)
{
	futex(futex_addr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, val, timeout, NULL, 0);
}

void muggle_futex_wake_one(muggle_atomic_int* futex_addr)
{
	futex(futex_addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0);
}

void muggle_futex_wake_all(muggle_atomic_int *futex_addr)
{
	futex(futex_addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, NULL, 0);
}


#endif
