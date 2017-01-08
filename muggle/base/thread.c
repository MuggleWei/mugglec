#include "thread.h"
#include "muggle/base/log.h"

#if MUGGLE_PLATFORM_WINDOWS

bool ThreadCreate(
	ThreadHandle *thread_handle, const ThreadAttribute *attr,
	ThreadStartRoutine routine, void *args)
{
	DWORD create_flags;
	LPSECURITY_ATTRIBUTES security_attr;

	MUGGLE_ASSERT_MSG(thread_handle != NULL, "Thread handle is NULL\n");
	if (thread_handle == NULL)
	{
		return false;
	}

	security_attr = (attr == NULL) ? (LPSECURITY_ATTRIBUTES)NULL : &attr->security;
	create_flags = (attr == NULL) ? 0 : attr->flags;

	thread_handle->handle = CreateThread(
		security_attr, 0, (PTHREAD_START_ROUTINE)routine, 
		(LPVOID)args, create_flags, &thread_handle->id);
	if (thread_handle->handle == NULL)
	{
		MUGGLE_DEBUG_WARNING("Failed create thread - error code %ld\n", (long)GetLastError());
		return false;
	}

	return true;
}

bool ThreadWaitExit(ThreadHandle *thread_handle)
{
	MUGGLE_ASSERT_MSG(thread_handle != NULL, "Thread handle is NULL\n");
	if (thread_handle == NULL)
	{
		return false;
	}

	if (WaitForSingleObject(thread_handle->handle, INFINITE) == WAIT_FAILED)
	{
		MUGGLE_DEBUG_WARNING("Failed wait thread exit - error code %ld\n", (long)GetLastError());
		return false;
	}

	if (!CloseHandle(thread_handle->handle))
	{
		MUGGLE_DEBUG_WARNING("Failed close thread handle - error code %ld\n", (long)GetLastError());
		return false;
	}

	return true;
}

void ThreadAttributeSet(int flags)
{
	// TODO: 
}

#else

#include <string.h>
#include <errno.h>

bool ThreadCreate(
	ThreadHandle *thread_handle, const ThreadAttribute *attr,
	ThreadStartRoutine routine, void *args)
{
	MUGGLE_ASSERT_MSG(thread_handle != NULL, "Thread handle is NULL\n");
	if (thread_handle == NULL)
	{
		return false;
	}

	if (pthread_create(&thread_handle->th, &attr->attr, routine, args) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed create thread: %s\n", strerror(errno));
		return false;
	}

	return true;
}

bool ThreadWaitExit(ThreadHandle *thread_handle)
{
	MUGGLE_ASSERT_MSG(thread_handle != NULL, "Thread handle is NULL\n");
	if (thread_handle == NULL)
	{
		return false;
	}

	if (pthread_join(thread_handle->th, NULL) != 0)
	{
		MUGGLE_DEBUG_WARNING("Failed pthread_join: %s\n", strerror(errno));
		return false;
	}

	return true;
}

void ThreadAttributeSet(int flags)
{
	// TODO: 
}

#endif