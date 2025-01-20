/******************************************************************************
 *  @file         flow_controller.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2025-01-20
 *  @copyright    Copyright 2025 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec flow controller
 *****************************************************************************/

#ifndef MUGGLE_C_FLOW_CONTROLLER
#define MUGGLE_C_FLOW_CONTROLLER

#include "muggle/c/base/macro.h"
#include "muggle/c/time/time_counter.h"

EXTERN_C_BEGIN

typedef struct {
	int64_t *arr;
	int64_t t;
	uint32_t n;
	uint32_t cursor;
	muggle_time_counter_t tc;
} muggle_flow_controller_t;

/**
 * @brief init flow controller
 *
 * @param flow_ctl         flow controller pointer
 * @param time_range_sec   time range in seconds
 * @param n                max count in time range
 * @param init_forward_sec init forward seconds
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_flow_ctl_init(muggle_flow_controller_t *flow_ctl,
						  int64_t time_range_sec, uint32_t n,
						  int64_t init_forward_sec);

/**
 * @brief destroy flow controller
 *
 * @param flow_ctl  flow controller pointer
 */
MUGGLE_C_EXPORT
void muggle_flow_ctl_destroy(muggle_flow_controller_t *flow_ctl);

/**
 * @brief check current ts, if check pass, update timestamp
 *
 * @param flow_ctl  flow controller pointer
 *
 * @return
 *   - true: flow control check pass
 *   - false: flow control check failed
 */
MUGGLE_C_EXPORT
bool muggle_flow_ctl_check_and_update(muggle_flow_controller_t *flow_ctl);

/**
 * @brief check current ts, always update timestamp
 *
 * @param flow_ctl  flow controller pointer
 *
 * @return
 *   - true: flow control check pass
 *   - false: flow control check failed
 */
MUGGLE_C_EXPORT
bool muggle_flow_ctl_check_and_force_update(muggle_flow_controller_t *flow_ctl);

/**
 * @brief get current time elapsed (nanoseconds)
 *
 * @param flow_ctl  flow controller pointer
 *
 * @return  time elapsed
 */
MUGGLE_C_EXPORT
int64_t muggle_flow_ctl_get_curr_elapsed(muggle_flow_controller_t *flow_ctl);

/**
 * @brief flow controller check
 *
 * @param flow_ctl    flow controller pointer
 * @param elapsed_ns  elapsed nanoseconds
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_flow_ctl_check(muggle_flow_controller_t *flow_ctl,
						   int64_t elapsed_ns);

/**
 * @brief flow controller update
 *
 * @param flow_ctl    flow controller pointer
 * @param elapsed_ns  elapsed nanoseconds
 */
MUGGLE_C_EXPORT
void muggle_flow_ctl_update(muggle_flow_controller_t *flow_ctl,
							int64_t elapsed_ns);

EXTERN_C_END

#endif // !MUGGLE_C_FLOW_CONTROLLER
