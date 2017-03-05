/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_LOCK_H_
#define MUGGLE_C_LOCK_H_

#include "muggle/base_c/macro.h"
#include <stdint.h>
#include <stdbool.h>

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

#else

#include <pthread.h>

#endif

EXTERN_C_BEGIN

enum eLockStatus
{
	MUGGLE_LOCK_UNLOCK = 0,
	MUGGLE_LOCK_LOCKED,
};

typedef struct MutexLockHandle_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	HANDLE mtx;
#else
	pthread_mutex_t mtx;
#endif
}MutexLockHandle;

typedef struct SpinLockHandle_tag
{
	volatile int64_t status;
}SpinLockHandle;

MUGGLE_BASE_C_EXPORT bool InitMutexLock(MutexLockHandle *lock);
MUGGLE_BASE_C_EXPORT bool DestroyMutexLock(MutexLockHandle *lock);
MUGGLE_BASE_C_EXPORT bool LockMutexLock(MutexLockHandle *lock);
MUGGLE_BASE_C_EXPORT bool TryLockMutexLock(MutexLockHandle *lock);
MUGGLE_BASE_C_EXPORT bool UnlockMutexLock(MutexLockHandle *lock);

MUGGLE_BASE_C_EXPORT bool InitSpinLock(SpinLockHandle *lock);
MUGGLE_BASE_C_EXPORT bool DestroySpinLock(SpinLockHandle *lock);
MUGGLE_BASE_C_EXPORT bool LockSpinLock(SpinLockHandle *lock);
MUGGLE_BASE_C_EXPORT bool TryLockSpinLock(SpinLockHandle *lock);
MUGGLE_BASE_C_EXPORT bool UnlockSpinLock(SpinLockHandle *lock);

EXTERN_C_END

#endif