/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

 #ifndef MUGGLE_C_DELTA_TIME_H_
 #define MUGGLE_C_DELTA_TIME_H_

#include "muggle/base_c/macro.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

typedef struct deltaTime_tag
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;
}deltaTime;

#else

typedef struct deltaTime_tag
{
	struct timeval start;
	struct timeval end;
}deltaTime;

#endif

MUGGLE_BASE_C_EXPORT void DTStart(deltaTime *dt);
MUGGLE_BASE_C_EXPORT void DTEnd(deltaTime *dt);
MUGGLE_BASE_C_EXPORT double DTGetElapsedMilliseconds(deltaTime *dt);

EXTERN_C_END

 #endif