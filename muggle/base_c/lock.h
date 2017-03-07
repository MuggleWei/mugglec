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

typedef struct MuggleMutexLock_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	CRITICAL_SECTION cs;
#else
	pthread_mutex_t mtx;
#endif
}MuggleMutexLock;

typedef struct MuggleSpinLock_tag
{
	volatile int64_t status;
}MuggleSpinLock;

MUGGLE_BASE_C_EXPORT bool MuggleInitMutexLock(MuggleMutexLock *lock);
MUGGLE_BASE_C_EXPORT bool MuggleDestroyMutexLock(MuggleMutexLock *lock);
MUGGLE_BASE_C_EXPORT bool MuggleLockMutexLock(MuggleMutexLock *lock);
MUGGLE_BASE_C_EXPORT bool MuggleTryLockMutexLock(MuggleMutexLock *lock);
MUGGLE_BASE_C_EXPORT bool MuggleUnlockMutexLock(MuggleMutexLock *lock);

MUGGLE_BASE_C_EXPORT bool MuggleInitSpinLock(MuggleSpinLock *lock);
MUGGLE_BASE_C_EXPORT bool MuggleDestroySpinLock(MuggleSpinLock *lock);
MUGGLE_BASE_C_EXPORT bool MuggleLockSpinLock(MuggleSpinLock *lock);
MUGGLE_BASE_C_EXPORT bool MuggleTryLockSpinLock(MuggleSpinLock *lock);
MUGGLE_BASE_C_EXPORT bool MuggleUnlockSpinLock(MuggleSpinLock *lock);

EXTERN_C_END

#endif