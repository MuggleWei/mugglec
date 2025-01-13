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
#include "muggle/c/base/sleep.h"
#include "muggle/c/time/time_counter.h"

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

double muggle_rdtsc_freq_calibrate()
{
#if MUGGLE_SUPPORT_RDTSC
	muggle_time_counter_t tc;
	muggle_time_counter_init(&tc);
	muggle_time_counter_start(&tc);
	uint64_t start_ticks = muggle_rdtsc();
	muggle_nsleep(2 * 1000 * 1000);
	uint64_t end_ticks = muggle_rdtsc();
	muggle_time_counter_end(&tc);

	uint64_t elapsed_ns = muggle_time_counter_interval_ns(&tc);
	uint64_t elapsed_ticks = end_ticks - start_ticks;

	double rdtsc_ticks_per_sec = (1e9 * elapsed_ticks) / elapsed_ns;
	return rdtsc_ticks_per_sec;
#else
	return -1.0;
#endif
}

double muggle_tsc_elapsed_ns(uint64_t start_ticks, uint64_t end_ticks,
							 double ticks_per_sec)
{
	return (1e9 * (end_ticks - start_ticks)) / ticks_per_sec;
}
