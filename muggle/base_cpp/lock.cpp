/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base_cpp/lock.h"
#include "muggle/base_c/atomic.h"

NS_MUGGLE_BEGIN

/*************** mutex lock ***************/
MutexLock::MutexLock()
{
#if MUGGLE_PLATFORM_WINDOWS
	InitializeCriticalSection(&cs_);
#else
	pthread_mutex_init(&mtx_, nullptr);
#endif
}
MutexLock::~MutexLock()
{
#if MUGGLE_PLATFORM_WINDOWS
	DeleteCriticalSection(&cs_);
#else
	pthread_mutex_destroy(&mtx_);
#endif
}

bool MutexLock::Lock()
{
#if MUGGLE_PLATFORM_WINDOWS
	EnterCriticalSection(&cs_);
	return true;
#else
	return pthread_mutex_lock(&mtx_) == 0;
#endif
}
bool MutexLock::Unlock()
{
#if MUGGLE_PLATFORM_WINDOWS
	LeaveCriticalSection(&cs_);
	return true;
#else
	return pthread_mutex_unlock(&mtx_) == 0;
#endif
}
bool MutexLock::TryLock()
{
#if MUGGLE_PLATFORM_WINDOWS
	return TryEnterCriticalSection(&cs_) == TRUE;
#else
	return pthread_mutex_trylock(&mtx_) == 0;
#endif
}


/*************** spin lock ***************/
SpinLock::SpinLock()
{
	status_ = UNLOCK;
}
SpinLock::~SpinLock()
{}

bool SpinLock::Lock()
{
	while (MUGGLE_ATOMIC_CAS_64(status_, UNLOCK, LOCKED) != UNLOCK)
	{
		continue;
	}
	return true;
}
bool SpinLock::Unlock()
{
	return MUGGLE_ATOMIC_CAS_64(status_, LOCKED, UNLOCK) == LOCKED;
}
bool SpinLock::TryLock()
{
	return MUGGLE_ATOMIC_CAS_64(status_, UNLOCK, LOCKED) == UNLOCK;
}


/*************** scope lock ***************/
ScopeLock::ScopeLock(ILock &rhs)
{
	lock_ = &rhs;
	lock_->Lock();
}
ScopeLock::~ScopeLock()
{
	lock_->Unlock();
}

NS_MUGGLE_END