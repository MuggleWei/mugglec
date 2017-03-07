/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base_c/cond.h"
#include "muggle/base_c/log.h"

#if MUGGLE_PLATFORM_WINDOWS

bool MuggleInitCondVar(MuggleCondVar *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	InitializeConditionVariable(&cond->cond_var);

	return true;
}
bool MuggleWaitCondVar(MuggleCondVar *cond, MuggleMutexLock *mtx, long wait_ms)
{
	DWORD ms;
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	ms = (wait_ms == -1) ? INFINITE : (DWORD)wait_ms;
	return SleepConditionVariableCS(&cond->cond_var, &mtx->cs, ms);
}
bool MuggleWakeCondVar(MuggleCondVar *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	WakeConditionVariable(&cond->cond_var);

	return true;
}
bool MuggleWakeAllCondVar(MuggleCondVar *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	WakeAllConditionVariable(&cond->cond_var);

	return true;
}
bool MuggleDestroyCondVar(MuggleCondVar *cond)
{
	// windows has not destroy condition varaible
	return true;
}

#else

bool MuggleInitCondVar(MuggleCondVar *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	return pthread_cond_init(&cond->cond_var, NULL) == 0;
}
bool MuggleWaitCondVar(MuggleCondVar *cond, MuggleMutexLock *mtx, long wait_ms)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	if (wait_ms == -1)
	{
		return pthread_cond_wait(&cond->cond_var, &mtx->mtx) == 0;
	}
	else
	{
		struct timespec ts;
		ts.tv_sec = time(NULL) + wait_ms / 1000;
		ts.tv_nsec = (wait_ms % 1000) * 1000000;
		return pthread_cond_timedwait(&cond->cond_var, &mtx->mtx, &ts);
	}
}
bool MuggleWakeCondVar(MuggleCondVar *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	return pthread_cond_signal(&cond->cond_var) == 0;
}
bool MuggleWakeAllCondVar(MuggleCondVar *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	return pthread_cond_broadcast(&cond->cond_var) == 0;
}
bool MuggleDestroyCondVar(MuggleCondVar *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	return pthread_cond_destroy(&cond->cond_var) == 0;
}

#endif