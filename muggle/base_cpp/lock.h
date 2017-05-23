/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_CPP_LOCK_H_
#define MUGGLE_CPP_LOCK_H_

#include "muggle/base_c/macro.h"
#include "muggle/base_cpp/base_cpp_macro.h"

#include <stdint.h>

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

#else

#include <pthread.h>

#endif

NS_MUGGLE_BEGIN


class MUGGLE_BASE_CPP_EXPORT ILock
{
public:
	virtual ~ILock() {}

	virtual bool Lock() = 0;
	virtual bool Unlock() = 0;
	virtual bool TryLock() = 0;
};


class MUGGLE_BASE_CPP_EXPORT MutexLock : public ILock
{
public:
	MutexLock();
	~MutexLock();

	MutexLock(const MutexLock&) = delete;
	MutexLock(const MutexLock&&) = delete;
	MutexLock& operator=(const MutexLock&) = delete;
	MutexLock& operator=(const MutexLock&&) = delete;

	virtual bool Lock() override;
	virtual bool Unlock() override;
	virtual bool TryLock() override;

private:
#if MUGGLE_PLATFORM_WINDOWS
	CRITICAL_SECTION cs_;
#else
	pthread_mutex_t mtx_;
#endif
};

class MUGGLE_BASE_CPP_EXPORT SpinLock : public ILock
{
public:
	SpinLock();
	~SpinLock();

	SpinLock(const SpinLock&) = delete;
	SpinLock(const SpinLock&&) = delete;
	SpinLock& operator=(const SpinLock&) = delete;
	SpinLock& operator=(const SpinLock&&) = delete;

	virtual bool Lock() override;
	virtual bool Unlock() override;
	virtual bool TryLock() override;

private:
	enum
	{
		UNLOCK = 0,
		LOCKED,
	};

	volatile int64_t status_;
};

class MUGGLE_BASE_CPP_EXPORT ScopeLock
{
public:
	ScopeLock(ILock &rhs);
	~ScopeLock();

private:
	ILock *lock_;
};

NS_MUGGLE_END

#endif