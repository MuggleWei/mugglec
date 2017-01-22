/*
*	author: muggle wei <mugglewei@gmail.com>
*
*	Use of this source code is governed by the MIT license that can be
*	found in the LICENSE file.
*/

#include "muggle/base/mutex.h"
#include "muggle/base/log.h"

#if MUGGLE_PLATFORM_WINDOWS

bool MutexInit(MutexHandle *mtx)
{
	MUGGLE_ASSERT_MSG(mtx != NULL, "Mutex handle is NULL\n");
	if (mtx == NULL)
	{
		return false;
	}

	mtx->mtx = CreateMutex(NULL, FALSE, NULL);
	if (mtx->mtx == NULL)
	{
		MUGGLE_DEBUG_WARNING("Failed create Mutex - error code %ld\n", (long)GetLastError());
		return false;
	}

	return true;
}
bool MutexDestroy(MutexHandle *mtx)
{
	MUGGLE_ASSERT_MSG(mtx != NULL, "Mutex handle is NULL\n");
	if (mtx == NULL)
	{
		return false;
	}

	if (!CloseHandle(mtx->mtx))
	{
		MUGGLE_DEBUG_WARNING("Failed close Mutex - error code %ld\n", (long)GetLastError());
		return false;
	}

	mtx->mtx = NULL;

	return true;
}
bool MutexLock(MutexHandle *mtx)
{
	MUGGLE_ASSERT_MSG(mtx != NULL, "Mutex handle is NULL\n");
	if (mtx == NULL)
	{
		return false;
	}

	if (WaitForSingleObject(mtx->mtx, INFINITE) == WAIT_FAILED)
	{
		MUGGLE_DEBUG_WARNING("Failed lock mutex - error code %ld\n", (long)GetLastError());
		return false;
	}

	return true;
}
bool MutexUnLock(MutexHandle *mtx)
{
	MUGGLE_ASSERT_MSG(mtx != NULL, "Mutex handle is NULL\n");
	if (mtx == NULL)
	{
		return false;
	}

	if (!ReleaseMutex(mtx->mtx))
	{
		MUGGLE_DEBUG_WARNING("Failed unlock mutex - error code %ld\n", (long)GetLastError());
		return false;
	}

	return true;
}

#else

#include <string.h>
#include <errno.h>

bool MutexInit(MutexHandle *mtx)
{
	MUGGLE_ASSERT_MSG(mtx != NULL, "Mutex handle is NULL\n");
	if (mtx == NULL)
	{
		return false;
	}

	if (pthread_mutex_init(&mtx->mtx, NULL) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed create Mutex - %s\n", strerror(errno));
		return false;
	}

	return true;
}
bool MutexDestroy(MutexHandle *mtx)
{
	MUGGLE_ASSERT_MSG(mtx != NULL, "Mutex handle is NULL\n");
	if (mtx == NULL)
	{
		return false;
	}

	if (pthread_mutex_destroy(&mtx->mtx) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed destroy Mutex - %s\n", strerror(errno));
		return false;
	}

	return true;
}
bool MutexLock(MutexHandle *mtx)
{
	MUGGLE_ASSERT_MSG(mtx != NULL, "Mutex handle is NULL\n");
	if (mtx == NULL)
	{
		return false;
	}

	if (pthread_mutex_lock(&mtx->mtx) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed lock Mutex - %s\n", strerror(errno));
		return false;
	}

	return true;
}
bool MutexUnLock(MutexHandle *mtx)
{
	MUGGLE_ASSERT_MSG(mtx != NULL, "Mutex handle is NULL\n");
	if (mtx == NULL)
	{
		return false;
	}

	if (pthread_mutex_unlock(&mtx->mtx) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed unlock Mutex - %s\n", strerror(errno));
		return false;
	}

	return true;
}

#endif