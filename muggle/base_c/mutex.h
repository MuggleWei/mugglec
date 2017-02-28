/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_MUTEX_H_
#define MUGGLE_C_MUTEX_H_

#include "muggle/base_c/macro.h"
#include <stdbool.h>

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

#else

#include <pthread.h>

#endif

EXTERN_C_BEGIN

typedef struct MutexHandle_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	HANDLE mtx;
#else
	pthread_mutex_t mtx;
#endif
}MutexHandle;

MUGGLE_BASE_C_EXPORT bool MutexInit(MutexHandle *mtx);
MUGGLE_BASE_C_EXPORT bool MutexDestroy(MutexHandle *mtx);
MUGGLE_BASE_C_EXPORT bool MutexLock(MutexHandle *mtx);
MUGGLE_BASE_C_EXPORT bool MutexUnLock(MutexHandle *mtx);

EXTERN_C_END

#endif