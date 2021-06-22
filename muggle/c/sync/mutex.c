/******************************************************************************
 *  @file         mutex.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec mutex
 *****************************************************************************/

#include "mutex.h"
#include "muggle/c/base/err.h"

#if MUGGLE_PLATFORM_WINDOWS

int muggle_mutex_init(muggle_mutex_t *mutex)
{
	InitializeCriticalSection(&mutex->cs);
	return MUGGLE_OK;
}

int muggle_mutex_destroy(muggle_mutex_t *mutex)
{
	DeleteCriticalSection(&mutex->cs);
	return MUGGLE_OK;
}

int muggle_mutex_lock(muggle_mutex_t *mutex)
{
	EnterCriticalSection(&mutex->cs);
	return MUGGLE_OK;
}

int muggle_mutex_trylock(muggle_mutex_t *mutex)
{
	if (!TryEnterCriticalSection(&mutex->cs))
	{
		return MUGGLE_ERR_ACQ_LOCK;
	}

	return MUGGLE_OK;
}

int muggle_mutex_unlock(muggle_mutex_t *mutex)
{
	LeaveCriticalSection(&mutex->cs);
	return MUGGLE_OK;
}

#else

int muggle_mutex_init(muggle_mutex_t *mutex)
{
	if (pthread_mutex_init(&mutex->mtx, NULL) != 0)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	return MUGGLE_OK;
}

int muggle_mutex_destroy(muggle_mutex_t *mutex)
{
	if (pthread_mutex_destroy(&mutex->mtx) != 0)
	{
		return MUGGLE_ERR_SYS_CALL;
	}
	return MUGGLE_OK;
}

int muggle_mutex_lock(muggle_mutex_t *mutex)
{
	if (pthread_mutex_lock(&mutex->mtx) != 0)
	{
		return MUGGLE_ERR_SYS_CALL;
	}
	return MUGGLE_OK;
}

int muggle_mutex_trylock(muggle_mutex_t *mutex)
{
	if (pthread_mutex_trylock(&mutex->mtx) != 0)
	{
		return MUGGLE_ERR_ACQ_LOCK;
	}
	return MUGGLE_OK;
}

int muggle_mutex_unlock(muggle_mutex_t *mutex)
{
	if (pthread_mutex_unlock(&mutex->mtx) != 0)
	{
		return MUGGLE_ERR_SYS_CALL;
	}
	return MUGGLE_OK;
}

#endif
