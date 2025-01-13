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

EXTERN_C_END

#endif
