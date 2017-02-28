/*
*	author: muggle wei <mugglewei@gmail.com>
*
*	Use of this source code is governed by the MIT license that can be
*	found in the LICENSE file.
*/

#include "muggle/base_c/sleep.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif

void SleepFunction(unsigned long ms)
{
#if MUGGLE_PLATFORM_WINDOWS
	Sleep(ms);
#else
	usleep((double)(ms) * 1000.0);
#endif
}