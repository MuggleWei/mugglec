/******************************************************************************
 *  @file         log_handler.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log handler
 *****************************************************************************/

#ifndef MUGGLE_C_LOG_HANDLE_H_
#define MUGGLE_C_LOG_HANDLE_H_

#include "muggle/c/base/macro.h"
#include <stdbool.h>
#include "muggle/c/sync/mutex.h"
#include "muggle/c/log/log_msg.h"
#include "muggle/c/log/log_fmt.h"

EXTERN_C_BEGIN

struct muggle_log_handler;

// prototype of log handler function
typedef int (*func_muggle_log_handler_write)(
	struct muggle_log_handler *handler, const muggle_log_msg_t *msg);

typedef int (*func_muggle_log_handler_destroy)(
	struct muggle_log_handler *handler);

/**
 * @brief muggle log handler
 */
typedef struct muggle_log_handler
{
	func_muggle_log_handler_write   write;
	func_muggle_log_handler_destroy destroy;

	int              level;
	muggle_log_fmt_t *fmt;
	bool             need_mutex;
	muggle_mutex_t   mtx;          //!< mutex
}muggle_log_handler_t;

/**
 * @brief set log handler formatter
 *
 * @param handler  log handler pointer
 * @param fmt      log formatter
 */
MUGGLE_C_EXPORT
void muggle_log_handler_set_fmt(muggle_log_handler_t *handler, muggle_log_fmt_t *fmt);

/**
 * @brief get log handler's formatter
 *
 * @param handler log handler pointer
 *
 * @return log formatter
 */
MUGGLE_C_EXPORT
muggle_log_fmt_t* muggle_log_handler_get_fmt(muggle_log_handler_t *handler);

/**
 * @brief set log handler level
 *
 * @param handler  log handler pointer
 * @param level    log level, see MUGGLE_LOG_LEVEL_*
 */
MUGGLE_C_EXPORT
void muggle_log_handler_set_level(muggle_log_handler_t *handler, int level);

/**
 * @brief get log handler's level
 *
 * @param handler  log handler pointer
 *
 * @return log handler's level
 */
MUGGLE_C_EXPORT
int muggle_log_handler_get_level(muggle_log_handler_t *handler);

/**
 * @brief detect level should write in log handler
 *
 * @param handler log handler pointer
 * @param level   log level
 */
MUGGLE_C_EXPORT
bool muggle_log_handler_should_write(muggle_log_handler_t *handler, int level);

/**
 * @brief set handler need mutex or not
 *
 * @param flag need mutex
 */
MUGGLE_C_EXPORT
void muggle_log_handler_set_mutex(muggle_log_handler_t *handler, bool flag);

/**
 * @brief intialize base log handler
 *
 * @param handler  log handler pointer
 *
 * @return
 *     - on success, return 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_handler_init_default(muggle_log_handler_t *handler);

/**
 * @brief destroy base log handler
 *
 * @param base_handler log handler pointer
 *
 * @return 
 *     - on success, return 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_handler_destroy_default(muggle_log_handler_t *handler);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_LOG_HANDLE_H_ */
