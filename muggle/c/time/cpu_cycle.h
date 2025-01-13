/******************************************************************************
 *  @file         cpu_cycle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec cpu cycle
 *****************************************************************************/

#ifndef MUGGLE_C_CPU_CYCLE_H_
#define MUGGLE_C_CPU_CYCLE_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS
	#if defined(_M_IX86) || defined(_M_X64)
		#define MUGGLE_SUPPORT_RDTSC 1
	#else
		#define MUGGLE_SUPPORT_RDTSC 0
	#endif
#elif MUGGLE_PLATFORM_LINUX && defined(__x86_64__)
	#define MUGGLE_SUPPORT_RDTSC 1
#else
	#define MUGGLE_SUPPORT_RDTSC 0
#endif

/**
 * @brief get cpu cycle
 *
 * @return number of clock cycles since the last reset
 */
MUGGLE_C_EXPORT
uint64_t muggle_get_cpu_cycle();

/**
 * @brief rdtsc
 *
 * @return
 *   on success, return cpu time Time Stamp Counter
 *   otherwise return 0
 *
 * @NOTE only support in x86/x64, check MUGGLE_SUPPORT_RDTSC
 */
MUGGLE_C_EXPORT
uint64_t muggle_rdtsc();

/**
 * @brief rdtscp
 *
 * @return
 *   on success, return cpu time Time Stamp Counter
 *   otherwise return 0
 *
 * @NOTE only support in x86/x64, check MUGGLE_SUPPORT_RDTSC
 */
MUGGLE_C_EXPORT
uint64_t muggle_rdtscp();

/**
 * @brief measure rdtsc ticks per second
 *
 * @return rdtsc ticks per second
 *
 * @NOTE
 *   1. only support in x86/x64, check MUGGLE_SUPPORT_RDTSC
 *   2. user need gurantee CPU support constant TSC
 */
MUGGLE_C_EXPORT
double muggle_rdtsc_freq_calibrate();

/**
 * @brief measure elapsed ns
 *
 * @param start_ticks    start tsc tick
 * @param end_ticks      end tsc tick
 * @param ticks_per_sec  tsc ticks per second
 *
 * @return 
 */
MUGGLE_C_EXPORT
double muggle_tsc_elapsed_ns(uint64_t start_ticks, uint64_t end_ticks,
							 double ticks_per_sec);

EXTERN_C_END

#endif
