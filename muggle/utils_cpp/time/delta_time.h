/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_DELTA_TIMER_H_
#define MUGGLE_DELTA_TIMER_H_

#include "muggle/base_c/macro.h"
#include "muggle/utils_cpp/utils_cpp_macro.h"

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

#else

#include <sys/time.h>

#endif


NS_MUGGLE_BEGIN

class MUGGLE_UTILS_CPP_EXPORT DeltaTime
{
public:
	DeltaTime() : start_(), end_() {}

	void Start();
	void End();
	double GetElapsedMilliseconds();

private:

#if MUGGLE_PLATFORM_WINDOWS
	LARGE_INTEGER start_;
	LARGE_INTEGER end_;
#else
	struct timeval start_;
	struct timeval end_;
#endif

};

NS_MUGGLE_END

#endif