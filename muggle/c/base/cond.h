/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_COND_H_
#define MUGGLE_C_COND_H_

#include "muggle/c/base/macro.h"
#if MUGGLE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#include <limits.h>	// for ULONG_MAX (MUGGLE_WAIT_INFINITE expand)
#endif
#include <stdbool.h>
#include "muggle/c/base/lock.h"

EXTERN_C_BEGIN

typedef struct MuggleCondVar_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	CONDITION_VARIABLE cond_var;
#else
	pthread_cond_t cond_var;
#endif
}MuggleCondVar;

MUGGLE_CC_EXPORT bool MuggleInitCondVar(MuggleCondVar *cond);
MUGGLE_CC_EXPORT bool MuggleWaitCondVar(MuggleCondVar *cond, MuggleMutexLock *mtx, unsigned long wait_ms);
MUGGLE_CC_EXPORT bool MuggleWakeCondVar(MuggleCondVar *cond);
MUGGLE_CC_EXPORT bool MuggleWakeAllCondVar(MuggleCondVar *cond);
MUGGLE_CC_EXPORT bool MuggleDestroyCondVar(MuggleCondVar *cond);

EXTERN_C_END

#endif