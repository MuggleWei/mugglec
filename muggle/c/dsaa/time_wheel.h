/******************************************************************************
 *  @file         time_wheel.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2026-06-17
 *  @copyright    Copyright 2026 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec time wheel
 *****************************************************************************/

#ifndef MUGGLE_C_DSAA_TIME_WHEEL_H_
#define MUGGLE_C_DSAA_TIME_WHEEL_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/time/time_counter.h"
#include <stdint.h>
#include <stdbool.h>

EXTERN_C_BEGIN

#define MUGGLE_TIME_WHEEL_INSERT_MODE_CURRENT 0
#define MUGGLE_TIME_WHEEL_INSERT_MODE_LOOP 1

/**
 * @brief time wheel node
 */
typedef struct muggle_time_wheel_node {
	struct muggle_time_wheel_node *prev;
	struct muggle_time_wheel_node *next;
	void *data;
} muggle_time_wheel_node_t;

/**
 * @brief time wheel slot
 */
typedef struct {
	muggle_time_wheel_node_t head;
	muggle_time_wheel_node_t tail;
} muggle_time_wheel_slot_t;

/**
 * @brief time wheel
 */
typedef struct {
	muggle_time_wheel_slot_t *slots; //!< time wheel slots
	uint32_t n; //!< number of slots
	uint32_t unit_ms; //!< unit time of one slot (milli-seconds)
	uint32_t insert_mode; //!< insert mode(MUGGLE_TIME_WHEEL_INSERT_MODE_*)
	muggle_time_counter_t tc; //!< time counter
	uint32_t cur; //!< current cursor
	uint32_t insert_cur; //!< for MUGGLE_TIME_WHEEL_INSERT_MODE_LOOP
} muggle_time_wheel_t;

/**
 * @brief time wheel update callback
 *
 * @param node       time wheel node
 * @param user_data  user data
 */
typedef void (*fn_muggle_time_wheel_cb)(muggle_time_wheel_node_t *node,
										void *user_data);

/**
 * @brief init time wheel
 *
 * @param wheel    pointer to time wheel
 * @param n        number of slot in time wheel
 * @param unit_ms  unit time of one slot (milli-seconds)
 *
 * @return 
 */
MUGGLE_C_EXPORT
bool muggle_time_wheel_init(muggle_time_wheel_t *wheel, uint32_t n,
							uint32_t unit_ms, uint32_t insert_mode);

/**
 * @brief destroy time wheel
 *
 * @param wheel  pointer to time wheel
 */
MUGGLE_C_EXPORT
void muggle_time_wheel_destroy(muggle_time_wheel_t *wheel);

/**
 * @brief update time wheel
 *
 * @param wheel      pointer to time wheel
 * @param cb         callback function
 * @param user_data  user data
 */
MUGGLE_C_EXPORT
void muggle_time_wheel_update(muggle_time_wheel_t *wheel,
							  fn_muggle_time_wheel_cb cb, void *user_data);

/**
 * @brief update n slot in time wheel
 *
 * @param wheel      pointer to time wheel
 * @param n_slot     number of slot
 * @param cb         callback function
 * @param user_data  user data
 */
MUGGLE_C_EXPORT
void muggle_time_wheel_update_n(muggle_time_wheel_t *wheel, uint32_t n_slot,
								fn_muggle_time_wheel_cb cb, void *user_data);

/**
 * @brief time wheel insert node
 *
 * @param wheel  pointer to time wheel
 * @param node   time wheel node
 */
MUGGLE_C_EXPORT
void muggle_time_wheel_insert(muggle_time_wheel_t *wheel,
							  muggle_time_wheel_node_t *node);

/**
 * @brief time wheel remove node
 *
 * @param wheel  pointer to time wheel
 * @param node   time wheel node
 */
MUGGLE_C_EXPORT
void muggle_time_wheel_remove(muggle_time_wheel_t *wheel,
							  muggle_time_wheel_node_t *node);

EXTERN_C_END

#endif // !MUGGLE_C_DSAA_TIME_WHEEL_H_
