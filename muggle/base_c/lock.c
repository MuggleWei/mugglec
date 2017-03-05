/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base_c/lock.h"
#include "muggle/base_c/log.h"
#include "muggle/base_c/atomic.h"

// mutex lock
#if MUGGLE_PLATFORM_WINDOWS

bool InitMutexLock(MutexLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	InitializeCriticalSection(&lock->cs);

	return true;
}
bool DestroyMutexLock(MutexLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	DeleteCriticalSection(&lock->cs);

	return true;
}
bool LockMutexLock(MutexLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	EnterCriticalSection(&lock->cs);

	return true;
}
bool TryLockMutexLock(MutexLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	return TryEnterCriticalSection(&lock->cs);
}
bool UnlockMutexLock(MutexLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	LeaveCriticalSection(&lock->cs);

	return true;
}

#else

#include <string.h>
#include <errno.h>

bool InitMutexLock(MutexLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	if (pthread_mutex_init(&lock->mtx, NULL) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed create Mutex - %s\n", strerror(errno));
		return false;
	}

	return true;
}
bool DestroyMutexLock(MutexLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	if (pthread_mutex_destroy(&lock->mtx) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed destroy Mutex - %s\n", strerror(errno));
		return false;
	}

	return true;
}
bool LockMutexLock(MutexLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	if (pthread_mutex_lock(&lock->mtx) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed lock Mutex - %s\n", strerror(errno));
		return false;
	}

	return true;
}
bool TryLockMutexLock(MutexLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	if (pthread_mutex_trylock(&lock->mtx) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed lock Mutex - %s\n", strerror(errno));
		return false;
	}

	return true;
}
bool UnlockMutexLock(MutexLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	if (pthread_mutex_unlock(&lock->mtx) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed unlock Mutex - %s\n", strerror(errno));
		return false;
	}

	return true;
}

#endif

// spin lock
bool InitSpinLock(SpinLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "SpinLock handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	lock->status = MUGGLE_LOCK_UNLOCK;

	return true;
}
bool DestroySpinLock(SpinLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "SpinLock handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	return true;
}
bool LockSpinLock(SpinLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "SpinLock handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	while (ATOMIC_CAS_64(lock->status, MUGGLE_LOCK_UNLOCK, MUGGLE_LOCK_LOCKED) != MUGGLE_LOCK_UNLOCK)
	{
		continue;
	}

	return true;
}
bool TryLockSpinLock(SpinLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "SpinLock handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	return ATOMIC_CAS_64(lock->status, MUGGLE_LOCK_UNLOCK, MUGGLE_LOCK_LOCKED) == MUGGLE_LOCK_UNLOCK;
}
bool UnlockSpinLock(SpinLockHandle *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "SpinLock handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	return ATOMIC_CAS_64(lock->status, MUGGLE_LOCK_LOCKED, MUGGLE_LOCK_UNLOCK) == MUGGLE_LOCK_LOCKED;
}