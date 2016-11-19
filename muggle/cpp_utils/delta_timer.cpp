/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/cpp_utils/delta_timer.h"
#include <stddef.h>

NS_MUGGLE_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

void DeltaTimer::Start()
{
	QueryPerformanceCounter(&start_);
}
void DeltaTimer::End()
{
	QueryPerformanceCounter(&end_);
}
double DeltaTimer::GetElapsedMilliseconds()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return (end_.QuadPart - start_.QuadPart) * 1000.0 / freq.QuadPart;
}

#else

void DeltaTimer::Start()
{
	gettimeofday(&start_, NULL);
}

void DeltaTimer::End()
{
	gettimeofday(&end_, NULL);
}

double DeltaTimer::GetElapsedMilliseconds()
{
	return (end_.tv_sec - start_.tv_sec) * 1000.0 + (end_.tv_usec - start_.tv_usec) / 1000.0;
}

#endif

NS_MUGGLE_END