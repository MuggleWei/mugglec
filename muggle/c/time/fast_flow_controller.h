/******************************************************************************
 *  @file         fast_flow_controller.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2025-01-20
 *  @copyright    Copyright 2025 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec fast flow controller
 *****************************************************************************/

#ifndef MUGGLE_C_FAST_FLOW_CONTROLLER_H_
#define MUGGLE_C_FAST_FLOW_CONTROLLER_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/time/cpu_cycle.h"
#include <stdbool.h>

EXTERN_C_BEGIN

#define MUGGLE_SUPPORT_FAST_FLOW_CONTROLLER MUGGLE_SUPPORT_RDTSC

typedef struct {
	int64_t *arr;
	int64_t t;
	int64_t t_ticks;
	uint32_t n;
	uint32_t cursor;
	uint64_t start_ticks;
} muggle_fast_flow_controller_t;

/**
 * @brief init fast flow controller
 *
 * @param flow_ctl         flow controller pointer
 * @param time_range_sec   time range in seconds
 * @param n                max count in time range
 * @param init_forward_sec init forward seconds
 * @param tick_freq        tick per seconds
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_fast_flow_ctl_init(muggle_fast_flow_controller_t *flow_ctl,
							   int64_t time_range_sec, uint32_t n,
							   int64_t init_forward_sec, double tick_freq);

/**
 * @brief destroy fast flow controller
 *
 * @param flow_ctl  fast flow controller pointer
 */
MUGGLE_C_EXPORT
void muggle_fast_flow_ctl_destroy(muggle_fast_flow_controller_t *flow_ctl);

/**
 * @brief check current ts, if check pass, update timestamp
 *
 * @param flow_ctl  flow controller pointer
 *
 * @return
 *   - true: check pass
 *   - false: check failed
 */
MUGGLE_C_EXPORT
bool muggle_fast_flow_ctl_check_and_update(
	muggle_fast_flow_controller_t *flow_ctl);

/**
 * @brief check current ts, always update timestamp
 *
 * @param flow_ctl  fast flow controller pointer
 *
 * @return
 *   - true: flow control check pass
 *   - false: flow control check failed
 */
MUGGLE_C_EXPORT
bool muggle_fast_flow_ctl_check_and_force_update(
	muggle_fast_flow_controller_t *flow_ctl);

/**
 * @brief get current time elapsed (ticks)
 *
 * @param flow_ctl  fast flow controller pointer
 *
 * @return  time elapsed (ticks)
 */
MUGGLE_C_EXPORT
int64_t
muggle_fast_flow_ctl_get_curr_elapsed(muggle_fast_flow_controller_t *flow_ctl);

/**
 * @brief fast flow controller check
 *
 * @param flow_ctl    fast flow controller pointer
 * @param elapsed_ns  elapsed ticks
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_fast_flow_ctl_check(muggle_fast_flow_controller_t *flow_ctl,
								int64_t elapsed_ticks);

/**
 * @brief fast flow controller update
 *
 * @param flow_ctl    fast flow controller pointer
 * @param elapsed_ns  elapsed ticks
 */
MUGGLE_C_EXPORT
void muggle_fast_flow_ctl_update(muggle_fast_flow_controller_t *flow_ctl,
								 int64_t elapsed_ticks);

EXTERN_C_END

#endif // !MUGGLE_C_FAST_FLOW_CONTROLLER_H_
