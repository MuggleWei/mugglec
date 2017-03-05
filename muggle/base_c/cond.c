#include "muggle/base_c/cond.h"
#include "muggle/base_c/log.h"

#if MUGGLE_PLATFORM_WINDOWS

bool InitCondVar(CondVarHandle *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	InitializeConditionVariable(&cond->cond_var);

	return true;
}
bool WaitCondVar(CondVarHandle *cond, MutexLockHandle *mtx, long wait_ms)
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
bool WakeCondVar(CondVarHandle *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	WakeConditionVariable(&cond->cond_var);

	return true;
}
bool WakeAllCondVar(CondVarHandle *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	WakeAllConditionVariable(&cond->cond_var);

	return true;
}
bool DestroyCondVar(CondVarHandle *cond)
{
	// windows has not destroy condition varaible
	return true;
}

#else

bool InitCondVar(CondVarHandle *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	return pthread_cond_init(&cond->cond_var, NULL) == 0;
}
bool WaitCondVar(CondVarHandle *cond, MutexLockHandle *mtx, long wait_ms)
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
bool WakeCondVar(CondVarHandle *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	return pthread_cond_signal(&cond->cond_var) == 0;
}
bool WakeAllCondVar(CondVarHandle *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	return pthread_cond_broadcast(&cond->cond_var) == 0;
}
bool DestroyCondVar(CondVarHandle *cond)
{
	MUGGLE_ASSERT_MSG(cond != NULL, "Condition variable pointer is NULL\n");
	if (cond == NULL)
	{
		return false;
	}

	return pthread_cond_destroy(&cond->cond_var) == 0;
}

#endif