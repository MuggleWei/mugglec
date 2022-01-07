#include "spinlock.h"
#include "muggle/c/base/thread.h"

void muggle_spinlock_init(muggle_atomic_int *spinlock)
{
	*spinlock = MUGGLE_SPINLOCK_STATUS_UNLOCK;
}

void muggle_spinlock_lock(muggle_atomic_int *spinlock)
{
	muggle_atomic_int expected = MUGGLE_SPINLOCK_STATUS_UNLOCK;
	while (!muggle_atomic_cmp_exch_weak(spinlock, &expected, MUGGLE_SPINLOCK_STATUS_LOCK, muggle_memory_order_acquire)
			&& expected != MUGGLE_SPINLOCK_STATUS_UNLOCK)
	{
		muggle_thread_yield();
		expected = MUGGLE_SPINLOCK_STATUS_UNLOCK;
	}
}

void muggle_spinlock_unlock(muggle_atomic_int *spinlock)
{
	muggle_atomic_store(spinlock, MUGGLE_SPINLOCK_STATUS_UNLOCK, muggle_memory_order_release);
}
