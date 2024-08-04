/******************************************************************************
 *  @file         cpu_cycle.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec cpu cycle
 *****************************************************************************/

#include "cpu_cycle.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <intrin.h>
#elif MUGGLE_PLATFORM_LINUX && defined(__x86_64__)
	#include <x86intrin.h>
#else
#endif

uint64_t muggle_get_cpu_cycle()
{
#if MUGGLE_PLATFORM_WINDOWS
	return __rdtsc();
#elif MUGGLE_PLATFORM_LINUX && defined(__x86_64__)
	return __rdtsc();
#else
	return 0;
#endif
}
