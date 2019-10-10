/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */


#include "thread.h"
#include "muggle/c/base/err.h"

#if MUGGLE_PLATFORM_WINDOWS

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

#else

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

#endif
