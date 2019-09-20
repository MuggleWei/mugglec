/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "mutex.h"
#include "muggle/c/base/err.h"

#if MUGGLE_PLATFORM_WINDOWS

int muggle_mutex_init(muggle_mutex_t *mutex)
{
	InitializeCriticalSection(&mutex->cs);
	return eMuggleOk;
}

int muggle_mutex_destroy(muggle_mutex_t *mutex)
{
	DeleteCriticalSection(&mutex->cs);
	return eMuggleOk;
}

int muggle_mutex_lock(muggle_mutex_t *mutex)
{
	EnterCriticalSection(&mutex->cs);
	return eMuggleOk;
}

int muggle_mutex_trylock(muggle_mutex_t *mutex)
{
	if (!TryEnterCriticalSection(&mutex->cs))
	{
		return eMuggleErrAcqLock;
	}

	return eMuggleOk;
}

int muggle_mutex_unlock(muggle_mutex_t *mutex)
{
	LeaveCriticalSection(&mutex->cs);
	return eMuggleOk;
}

#else

int muggle_mutex_init(muggle_mutex_t *mutex)
{
	if (pthread_mutex_init(&mutex->mtx, NULL) != 0)
	{
		return eMuggleErrSysCall;
	}

	return eMuggleOk;
}

int muggle_mutex_destroy(muggle_mutex_t *mutex)
{
	if (pthread_mutex_destroy(&mutex->mtx) != 0)
	{
		return eMuggleErrSysCall;
	}
	return eMuggleOk;
}

int muggle_mutex_lock(muggle_mutex_t *mutex)
{
	if (pthread_mutex_lock(&mutex->mtx) != 0)
	{
		return eMuggleErrSysCall;
	}
	return eMuggleOk;
}

int muggle_mutex_trylock(muggle_mutex_t *mutex)
{
	if (pthread_mutex_trylock(&mutex->mtx) != 0)
	{
		return eMuggleErrAcqLock;
	}
	return eMuggleOk;
}

int muggle_mutex_unlock(muggle_mutex_t *mutex)
{
	if (pthread_mutex_unlock(&mutex->mtx) != 0)
	{
		return eMuggleErrSysCall;
	}
	return eMuggleOk;
}

#endif
