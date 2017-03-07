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

typedef struct MuggleCondVar_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	CONDITION_VARIABLE cond_var;
#else
	pthread_cond_t cond_var;
#endif
}MuggleCondVar;

MUGGLE_BASE_C_EXPORT bool MuggleInitCondVar(MuggleCondVar *cond);
MUGGLE_BASE_C_EXPORT bool MuggleWaitCondVar(MuggleCondVar *cond, MuggleMutexLock *mtx, long wait_ms);
MUGGLE_BASE_C_EXPORT bool MuggleWakeCondVar(MuggleCondVar *cond);
MUGGLE_BASE_C_EXPORT bool MuggleWakeAllCondVar(MuggleCondVar *cond);
MUGGLE_BASE_C_EXPORT bool MuggleDestroyCondVar(MuggleCondVar *cond);

EXTERN_C_END

#endif