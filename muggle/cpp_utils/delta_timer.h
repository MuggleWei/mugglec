/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef __MUGGLE_DELTA_TIMER_H__
#define __MUGGLE_DELTA_TIMER_H__

#include "muggle/base/macro.h"
#include "muggle/cpp_utils/cpp_utils_macro.h"

NS_MUGGLE_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

class MUGGLE_CPP_UTLS_EXPORT DeltaTimer
{
public:
	DeltaTimer() : start_(), end_() {}

	void Start();
	void End();
	double GetElapsedMilliseconds();

private:
	LARGE_INTEGER start_;
	LARGE_INTEGER end_;
};

#else

#include <sys/time.h>

class DeltaTimer
{
public:
	DeltaTimer() : start_(), end_() {}

	void Start();
	void End();
	double GetElapsedMilliseconds();

private:
	struct timeval start_;
	struct timeval end_;
};

#endif

NS_MUGGLE_END

#endif