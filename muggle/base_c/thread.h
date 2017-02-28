/*
*	author: muggle wei <mugglewei@gmail.com>
*
*	Use of this source code is governed by the MIT license that can be
*	found in the LICENSE file.
*/

#ifndef __MUGGLE_THREAD_H__
#define __MUGGLE_THREAD_H__

#include "muggle/base_c/macro.h"
#include <stdbool.h>

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

#else

#include <pthread.h>

#endif

EXTERN_C_BEGIN

typedef struct ThreadHandle_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	HANDLE handle;
	DWORD id;
#else
	pthread_t th;
#endif
}ThreadHandle;

typedef struct ThreadAttribute_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	SECURITY_ATTRIBUTES security;
	DWORD flags;
#else
	pthread_attr_t attr;
#endif
}ThreadAttribute;

typedef void* (*ThreadStartRoutine)(void *args);

MUGGLE_BASE_C_EXPORT bool ThreadCreate(
	ThreadHandle *thread_handle, const ThreadAttribute *attr,
	ThreadStartRoutine routine, void *args);
MUGGLE_BASE_C_EXPORT bool ThreadWaitExit(ThreadHandle *thread_handle);

MUGGLE_BASE_C_EXPORT void ThreadAttributeSet(int flags);

EXTERN_C_END

#endif