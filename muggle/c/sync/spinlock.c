#include "spinlock.h"
#include "muggle/c/base/thread.h"

enum
{
	MUGGLE_SPINLOCK_STATUS_UNLOCK,
	MUGGLE_SPINLOCK_STATUS_LOCK,
};

void muggle_spinlock_init(muggle_spinlock_t *spinlock)
{
	*spinlock = MUGGLE_SPINLOCK_STATUS_UNLOCK;
}

void muggle_spinlock_lock(muggle_spinlock_t *spinlock)
{
	// muggle_atomic_int expected = MUGGLE_SPINLOCK_STATUS_UNLOCK;
	// while (!muggle_atomic_cmp_exch_weak(spinlock, &expected, MUGGLE_SPINLOCK_STATUS_LOCK, muggle_memory_order_acquire)
	//         && expected != MUGGLE_SPINLOCK_STATUS_UNLOCK)
	// {
	//     muggle_thread_yield();
	//     expected = MUGGLE_SPINLOCK_STATUS_UNLOCK;
	// }

	while (!muggle_atomic_test_and_set(spinlock, muggle_memory_order_acquire))
	{
		muggle_thread_yield();
	}
}

void muggle_spinlock_unlock(muggle_spinlock_t *spinlock)
{
	// muggle_atomic_store(spinlock, MUGGLE_SPINLOCK_STATUS_UNLOCK, muggle_memory_order_release);

	muggle_atomic_clear(spinlock, muggle_memory_order_release);
}
