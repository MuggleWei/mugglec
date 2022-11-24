#include "sync_obj.h"

#if MUGGLE_C_HAVE_LINUX_FUTEX || MUGGLE_C_HAVE_SYS_FUTEX

#if MUGGLE_C_HAVE_LINUX_FUTEX
	#include <linux/futex.h>
#elif MUGGLE_C_HAVE_SYS_FUTEX
	#include <sys/futex.h>
#endif
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>

long muggle_futex(
	muggle_sync_t *uaddr,
	int futex_op,
	muggle_sync_t val,
	const struct timespec *timeout,
	muggle_sync_t *uaddr2,
	muggle_sync_t val3)
{
	return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

int muggle_sync_wait(muggle_sync_t *addr, muggle_sync_t val, const struct timespec *timeout)
{
	return muggle_futex(
		addr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, val, timeout, NULL, 0);
}

int muggle_sync_wake_one(muggle_sync_t *addr)
{
	return muggle_futex(
		addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0);
}

int muggle_sync_wake_all(muggle_sync_t *addr)
{
	return muggle_futex(
		addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, NULL, 0);
}

#endif
