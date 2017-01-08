/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base_cpp/delta_time.h"
#include <stddef.h>

NS_MUGGLE_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

void DeltaTime::Start()
{
	QueryPerformanceCounter(&start_);
}
void DeltaTime::End()
{
	QueryPerformanceCounter(&end_);
}
double DeltaTime::GetElapsedMilliseconds()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return (end_.QuadPart - start_.QuadPart) * 1000.0 / freq.QuadPart;
}

#else

void DeltaTime::Start()
{
	gettimeofday(&start_, NULL);
}

void DeltaTime::End()
{
	gettimeofday(&end_, NULL);
}

double DeltaTime::GetElapsedMilliseconds()
{
	return (end_.tv_sec - start_.tv_sec) * 1000.0 + (end_.tv_usec - start_.tv_usec) / 1000.0;
}

#endif

NS_MUGGLE_END