/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "futex.h"
#include <sys/syscall.h>
#include <linux/futex.h>
#include <time.h>

#if MUGGLE_PLATFORM_WINDOWS
#else

static int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
	return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
}

void muggle_futex_wait(muggle_atomic_int* futex_addr, muggle_atomic_int val)
{
	futex(futex_addr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, val, NULL, NULL, 0); 
}

void muggle_futex_wake(muggle_atomic_int* futex_addr)
{
	futex(futex_addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0); 
}


#endif
