/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "cpu_cycle.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <intrin.h>
#elif MUGGLE_PLATFORM_LINUX
	#include <x86intrin.h>
#else
#endif

uint64_t muggle_get_cpu_cycle()
{
#if MUGGLE_PLATFORM_WINDOWS
	return __rdtsc();
#elif MUGGLE_PLATFORM_LINUX
	return __rdtsc();
#else
	return 0;
#endif
}
