/******************************************************************************
 *  @file         time_counter.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-11-29
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec time counter
 *****************************************************************************/

#ifndef MUGGLE_C_TIME_COUNTER_H_
#define MUGGLE_C_TIME_COUNTER_H_

#include "muggle/c/base/macro.h"
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#endif

EXTERN_C_BEGIN

typedef struct {
#if MUGGLE_PLATFORM_WINDOWS
	LARGE_INTEGER frequency;
	LARGE_INTEGER start;
	LARGE_INTEGER end;
#else
	struct timespec start_ts;
	struct timespec end_ts;
#endif
} muggle_time_counter_t;

/**
 * @brief initialize time counter
 *
 * @param tc  time counter
 *
 * @return  boolean
 */
MUGGLE_C_EXPORT
bool muggle_time_counter_init(muggle_time_counter_t *tc);

/**
 * @brief recording start time
 *
 * @param tc  time counter
 */
MUGGLE_C_EXPORT
void muggle_time_counter_start(muggle_time_counter_t *tc);

/**
 * @brief recording end time
 *
 * @param tc  time counter
 */
MUGGLE_C_EXPORT
void muggle_time_counter_end(muggle_time_counter_t *tc);

/**
 * @brief get time interval in nanoseconds
 *
 * @param tc  time counter
 *
 * @return  time interval in nanoseconds
 */
MUGGLE_C_EXPORT
int64_t muggle_time_counter_interval_ns(muggle_time_counter_t *tc);

/**
 * @brief get time interval in milliseconds
 *
 * @param tc  time counter
 *
 * @return  time elapsed in milliseconds
 */
MUGGLE_C_EXPORT
int64_t muggle_time_counter_interval_ms(muggle_time_counter_t *tc);

EXTERN_C_END

#endif // !MUGGLE_C_TIME_COUNTER_H_
