/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

 #ifndef MUGGLE_C_DELTA_TIME_H_
 #define MUGGLE_C_DELTA_TIME_H_

#include "muggle/base_c/macro.h"
#include "muggle/utils_c/utils_c_macro.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

typedef struct MuggleDeltaTime_tag
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;
}MuggleDeltaTime;

#else

typedef struct MuggleDeltaTime_tag
{
	struct timeval start;
	struct timeval end;
}MuggleDeltaTime;

#endif

MUGGLE_UTILS_C_EXPORT void MuggleDeltaTimeStart(MuggleDeltaTime *dt);
MUGGLE_UTILS_C_EXPORT void MuggleDeltaTimeEnd(MuggleDeltaTime *dt);
MUGGLE_UTILS_C_EXPORT double MuggleGetElapsedMilliseconds(MuggleDeltaTime *dt);

EXTERN_C_END

 #endif