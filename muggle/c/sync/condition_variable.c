#include "condition_variable.h"
#include "muggle/c/base/err.h"

#if MUGGLE_PLATFORM_WINDOWS

int muggle_condition_variable_init(muggle_condition_variable_t *cv)
{
	InitializeConditionVariable(&cv->cond_var);
	return MUGGLE_OK;
}

int muggle_condition_variable_destroy(muggle_condition_variable_t *cv)
{
	// windows has not destroy condition varaible
	return true;
}

int muggle_condition_variable_wait(muggle_condition_variable_t *cv, muggle_mutex_t *mutex, const struct timespec *timeout)
{
	BOOL ret;
	if (timeout == NULL)
	{
		ret = SleepConditionVariableCS(&cv->cond_var, &mutex->cs, INFINITE);
	}
	else
	{
		DWORD ms = timeout->tv_nsec / 1000000;
		if (ms == 0 && timeout->tv_nsec != 0)
		{
			ms = 1;
		}
		DWORD dwMilliseconds = timeout->tv_sec * 1000 + ms;
		ret = SleepConditionVariableCS(&cv->cond_var, &mutex->cs, dwMilliseconds);
	}
	return ret ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_condition_variable_notify_one(muggle_condition_variable_t *cv)
{
	WakeConditionVariable(&cv->cond_var);
	return MUGGLE_OK;
}

int muggle_condition_variable_notify_all(muggle_condition_variable_t *cv)
{
	WakeAllConditionVariable(&cv->cond_var);
	return MUGGLE_OK;
}

#else

#include <errno.h>

int muggle_condition_variable_init(muggle_condition_variable_t *cv)
{
	int ret = pthread_cond_init(&cv->cond_var, NULL);
	return ret == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_condition_variable_destroy(muggle_condition_variable_t *cv)
{
	int ret = pthread_cond_destroy(&cv->cond_var);
	return ret == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_condition_variable_wait(muggle_condition_variable_t *cv, muggle_mutex_t *mutex, const struct timespec *timeout)
{
	int ret;
	if (timeout == NULL)
	{
		ret = pthread_cond_wait(&cv->cond_var, &mutex->mtx);
		return ret == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
	}
	else
	{
		ret = pthread_cond_timedwait(&cv->cond_var, &mutex->mtx, timeout);
		if (ret == 0 || ret == ETIMEDOUT)
		{
			return MUGGLE_OK;
		}
		return MUGGLE_ERR_SYS_CALL;
	}
}

int muggle_condition_variable_notify_one(muggle_condition_variable_t *cv)
{
	int ret = pthread_cond_signal(&cv->cond_var);
	return ret == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_condition_variable_notify_all(muggle_condition_variable_t *cv)
{
	int ret = pthread_cond_broadcast(&cv->cond_var);
	return ret == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

#endif
