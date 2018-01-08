/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_THREAD_H_
#define MUGGLE_C_THREAD_H_

#include "muggle/c/base/macro.h"
#include <stdbool.h>

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

#define THREAD_ROUTINE_RETURN unsigned int

#else

#include <pthread.h>

#define THREAD_ROUTINE_RETURN void*

#endif

EXTERN_C_BEGIN

typedef struct MuggleThread_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	HANDLE handle;
	unsigned int id;
#else
	pthread_t th;
#endif
}MuggleThread;

typedef struct ThreadAttribute_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	SECURITY_ATTRIBUTES security;
	DWORD flags;
#else
	pthread_attr_t attr;
#endif
}MuggleThreadAttribute;

#if MUGGLE_PLATFORM_WINDOWS
typedef THREAD_ROUTINE_RETURN (__stdcall *MuggleThreadStartRoutine)(void *args);
#else
typedef THREAD_ROUTINE_RETURN (*MuggleThreadStartRoutine)(void *args);
#endif

MUGGLE_CC_EXPORT bool MuggleThreadCreate(
	MuggleThread *thread_handle, const MuggleThreadAttribute *attr,
	MuggleThreadStartRoutine routine, void *args);
MUGGLE_CC_EXPORT bool MuggleThreadWaitExit(MuggleThread *thread_handle);

MUGGLE_CC_EXPORT void MuggleThreadAttributeSet(int flags);

EXTERN_C_END

#endif