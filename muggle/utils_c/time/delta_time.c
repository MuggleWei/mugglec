/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/utils_c/time/delta_time.h"
#include <stdlib.h>
#include <stddef.h>

#if MUGGLE_PLATFORM_WINDOWS

void MuggleDeltaTimeStart(MuggleDeltaTime *dt)
{
	QueryPerformanceCounter(&dt->start);
}
void MuggleDeltaTimeEnd(MuggleDeltaTime *dt)
{
	QueryPerformanceCounter(&dt->end);
}
double MuggleGetElapsedMilliseconds(MuggleDeltaTime *dt)
{
	LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return (dt->end.QuadPart - dt->start.QuadPart) * 1000.0 / freq.QuadPart;
}

#else

void MuggleDeltaTimeStart(MuggleDeltaTime *dt)
{
	gettimeofday(&dt->start, NULL);
}
void MuggleDeltaTimeEnd(MuggleDeltaTime *dt)
{
	gettimeofday(&dt->end, NULL);
}
double MuggleGetElapsedMilliseconds(MuggleDeltaTime *dt)
{
	return (dt->end.tv_sec - dt->start.tv_sec) * 1000.0 + (dt->end.tv_usec - dt->start.tv_usec) / 1000.0;
}

#endif