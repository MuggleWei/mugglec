/******************************************************************************
 *  @file         thread.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec thread
 *****************************************************************************/

#include "thread.h"
#include "muggle/c/base/err.h"

#if MUGGLE_PLATFORM_WINDOWS

#include <process.h>

int muggle_thread_create(muggle_thread_t *thread, muggle_thread_routine routine, void *args)
{
	thread->handle = (HANDLE)_beginthreadex(
		NULL, 0, routine, args, 0, &thread->id);
	if (thread->handle == NULL)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	return MUGGLE_OK;
}

int muggle_thread_join(muggle_thread_t *thread)
{
	if (WaitForSingleObject(thread->handle, INFINITE) == WAIT_FAILED)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	if (!CloseHandle(thread->handle))
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	return MUGGLE_OK;
}

int muggle_thread_detach(muggle_thread_t *thread)
{
	return MUGGLE_OK;
}

muggle_thread_id muggle_thread_current_id()
{
	return GetCurrentThreadId();
}

muggle_thread_readable_id muggle_thread_current_readable_id()
{
	return GetCurrentThreadId();
}

int muggle_thread_hardware_concurrency()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return (int)sysinfo.dwNumberOfProcessors;
}

void muggle_thread_yield()
{
	SwitchToThread();
}

bool muggle_thread_equal(muggle_thread_id t1, muggle_thread_id t2)
{
	return t1 == t2;
}

#else

#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>

int muggle_thread_create(muggle_thread_t *thread, muggle_thread_routine routine, void *args)
{
	return pthread_create(&thread->th, NULL, routine, args) == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_thread_join(muggle_thread_t *thread)
{
	return pthread_join(thread->th, NULL) == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_thread_detach(muggle_thread_t *thread)
{
	return pthread_detach(thread->th) == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

muggle_thread_id muggle_thread_current_id()
{
	return pthread_self();
}

muggle_thread_readable_id muggle_thread_current_readable_id()
{
#if MUGGLE_PLATFORM_LINUX
	return syscall(SYS_gettid);
#elif MUGGLE_PLATFORM_APPLE
	uint64_t tid;
	pthread_threadid_np(NULL, &tid);
	return tid;
#elif MUGGLE_PLATFORM_FREEBSD
	long tid;
	thr_self(&tid);
	return (int)tid;
#else
	return syscall(SYS_gettid);
#endif
}

int muggle_thread_hardware_concurrency()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void muggle_thread_yield()
{
	sched_yield();
}

bool muggle_thread_equal(muggle_thread_id t1, muggle_thread_id t2)
{
	return pthread_equal(t1, t2);
}

#endif
