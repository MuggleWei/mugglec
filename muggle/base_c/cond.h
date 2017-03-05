/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_COND_H_
#define MUGGLE_C_COND_H_

#include "muggle/base_c/macro.h"
#if MUGGLE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <stdbool.h>
#include "muggle/base_c/lock.h"

EXTERN_C_BEGIN

typedef struct CondVarHandle_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	CONDITION_VARIABLE cond_var;
#else
	pthread_cond_t cond_var;
#endif
}CondVarHandle;

MUGGLE_BASE_C_EXPORT bool InitCondVar(CondVarHandle *cond);
MUGGLE_BASE_C_EXPORT bool WaitCondVar(CondVarHandle *cond, MutexLockHandle *mtx, long wait_ms);
MUGGLE_BASE_C_EXPORT bool WakeCondVar(CondVarHandle *cond);
MUGGLE_BASE_C_EXPORT bool WakeAllCondVar(CondVarHandle *cond);
MUGGLE_BASE_C_EXPORT bool DestroyCondVar(CondVarHandle *cond);

EXTERN_C_END

#endif