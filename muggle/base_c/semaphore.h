/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_SEMAPHORE_H_
#define MUGGLE_C_SEMAPHORE_H_

#include "muggle/base_c/macro.h"
#if MUGGLE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <limits.h>	// for ULONG_MAX (MUGGLE_WAIT_INFINITE expand)
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#endif
#include <stdbool.h>

EXTERN_C_BEGIN

enum eSemaphoreFlags
{
	MUGGLE_SEMAPHORE_CREAT	= 0x01,	// create semaphore if not exist
	MUGGLE_SEMAPHORE_EXCL	= 0x02,	// ensure this call create semaphore
};

typedef struct MuggleSemaphore_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	HANDLE sem;
#else
	int sem;
#endif
}MuggleSemaphore;

MUGGLE_BASE_C_EXPORT
bool MuggleOpenSemaphore(MuggleSemaphore *sem, const char *name, int flags, int attr, int init_val);
MUGGLE_BASE_C_EXPORT
bool MugglePostSemaphore(MuggleSemaphore *sem, int add_cnt);
MUGGLE_BASE_C_EXPORT
bool MuggleWaitSemaphore(MuggleSemaphore *sem);
MUGGLE_BASE_C_EXPORT
bool MuggleDestroySemaphore(MuggleSemaphore *sem);

EXTERN_C_END

#endif