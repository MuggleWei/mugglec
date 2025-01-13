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
	// see: https://learn.microsoft.com/en-us/cpp/intrinsics/compiler-intrinsics
	#include <intrin.h>
#elif MUGGLE_PLATFORM_LINUX && defined(__x86_64__)
	#include <x86intrin.h>
#else
#endif

uint64_t muggle_get_cpu_cycle()
{
#if MUGGLE_SUPPORT_RDTSC
	return __rdtsc();
#else
	return 0;
#endif
}

uint64_t muggle_rdtsc()
{
#if MUGGLE_PLATFORM_WINDOWS
	#if defined(_M_IX86) || defined(_M_X64)
	return __rdtsc();
	#else
	return 0;
	#endif
#elif MUGGLE_PLATFORM_LINUX && defined(__x86_64__)
	return __rdtsc();
#else
	return 0;
#endif
}

uint64_t muggle_rdtscp()
{
#if MUGGLE_PLATFORM_WINDOWS
	#if defined(_M_IX86) || defined(_M_X64)
	unsigned int tsc_aux;
	return __rdtscp(&tsc_aux);
	#else
	return 0;
	#endif
#elif MUGGLE_PLATFORM_LINUX && defined(__x86_64__)
	unsigned int tsc_aux;
	return __rdtscp(&tsc_aux);
#else
	return 0;
#endif
}
