#include "synclock.h"
#include "muggle/c/base/atomic.h"

#if MUGGLE_C_HAVE_SYNC_OBJ

enum
{
	MUGGLE_SYNCLOCK_STATUS_UNLOCK = 0,
	MUGGLE_SYNCLOCK_STATUS_LOCK,
};

void muggle_synclock_init(muggle_sync_t *synclock)
{
	*synclock = MUGGLE_SYNCLOCK_STATUS_UNLOCK;
}

void muggle_synclock_lock(muggle_sync_t *synclock)
{
	muggle_sync_t expected = MUGGLE_SYNCLOCK_STATUS_UNLOCK;
	while (!muggle_atomic_cmp_exch_weak(
		synclock, &expected, MUGGLE_SYNCLOCK_STATUS_LOCK,
		muggle_memory_order_acquire)
		&& expected != MUGGLE_SYNCLOCK_STATUS_UNLOCK)
	{
		muggle_sync_wait(synclock, expected, NULL);
		expected = MUGGLE_SYNCLOCK_STATUS_UNLOCK;
	}
}

void muggle_synclock_unlock(muggle_sync_t *synclock)
{
	muggle_atomic_store(
		synclock, MUGGLE_SYNCLOCK_STATUS_UNLOCK, muggle_memory_order_release);
	muggle_sync_wake_one(synclock);
}

#endif // MUGGLE_C_HAVE_SYNC_OBJ
