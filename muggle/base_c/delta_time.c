/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base_c/delta_time.h"
#include <stdlib.h>
#include <stddef.h>

#if MUGGLE_PLATFORM_WINDOWS

void DTStart(deltaTime *dt)
{
	QueryPerformanceCounter(&dt->start);
}
void DTEnd(deltaTime *dt)
{
	QueryPerformanceCounter(&dt->end);
}
double DTGetElapsedMilliseconds(deltaTime *dt)
{
	LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return (dt->end.QuadPart - dt->start.QuadPart) * 1000.0 / freq.QuadPart;
}

#else

void DTStart(deltaTime *dt)
{
	gettimeofday(&dt->start, NULL);
}
void DTEnd(deltaTime *dt)
{
	gettimeofday(&dt->end, NULL);
}
double DTGetElapsedMilliseconds(deltaTime *dt)
{
	return (dt->end.tv_sec - dt->start.tv_sec) * 1000.0 + (dt->end.tv_usec - dt->start.tv_usec) / 1000.0;
}

#endif