/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/c/base/lock.h"
#include "muggle/c/base/log.h"
#include "muggle/c/base/atomic.h"

// mutex lock
#if MUGGLE_PLATFORM_WINDOWS

bool MuggleInitMutexLock(MuggleMutexLock *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	InitializeCriticalSection(&lock->cs);

	return true;
}
bool MuggleDestroyMutexLock(MuggleMutexLock *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	DeleteCriticalSection(&lock->cs);

	return true;
}
bool MuggleLockMutexLock(MuggleMutexLock *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	EnterCriticalSection(&lock->cs);

	return true;
}
bool MuggleTryLockMutexLock(MuggleMutexLock *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "Mutex handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	return TryEnterCriticalSection(&lock->cs);
}
bool MuggleUnlockMutexLock(MuggleMutexLock *lock)
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

bool MuggleInitMutexLock(MuggleMutexLock *lock)
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
bool MuggleDestroyMutexLock(MuggleMutexLock *lock)
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
bool MuggleLockMutexLock(MuggleMutexLock *lock)
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
bool MuggleTryLockMutexLock(MuggleMutexLock *lock)
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
bool MuggleUnlockMutexLock(MuggleMutexLock *lock)
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
bool MuggleInitSpinLock(MuggleSpinLock *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "SpinLock handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	lock->status = MUGGLE_LOCK_UNLOCK;

	return true;
}
bool MuggleDestroySpinLock(MuggleSpinLock *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "SpinLock handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	return true;
}
bool MuggleLockSpinLock(MuggleSpinLock *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "SpinLock handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	while (MUGGLE_ATOMIC_CAS_64(lock->status, MUGGLE_LOCK_UNLOCK, MUGGLE_LOCK_LOCKED) != MUGGLE_LOCK_UNLOCK)
	{
		continue;
	}

	return true;
}
bool MuggleTryLockSpinLock(MuggleSpinLock *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "SpinLock handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	return MUGGLE_ATOMIC_CAS_64(lock->status, MUGGLE_LOCK_UNLOCK, MUGGLE_LOCK_LOCKED) == MUGGLE_LOCK_UNLOCK;
}
bool MuggleUnlockSpinLock(MuggleSpinLock *lock)
{
	MUGGLE_ASSERT_MSG(lock != NULL, "SpinLock handle is NULL\n");
	if (lock == NULL)
	{
		return false;
	}

	return MUGGLE_ATOMIC_CAS_64(lock->status, MUGGLE_LOCK_LOCKED, MUGGLE_LOCK_UNLOCK) == MUGGLE_LOCK_LOCKED;
}