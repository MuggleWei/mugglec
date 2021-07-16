/******************************************************************************
 *  @file         log_logger.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log logger
 *****************************************************************************/

#ifndef MUGGLE_C_LOG_LOGGER_H_
#define MUGGLE_C_LOG_LOGGER_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/log/log_handler.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_LOGGER_MAX_HANDLER = 8,
};

/**
 * @brief muggle logger
 */
typedef struct muggle_logger
{
	muggle_log_handler_t *handlers[MUGGLE_LOGGER_MAX_HANDLER];
	int cnt;
	int fmt_hint;
	int lowest_log_level;
}muggle_logger_t;

/**
 * @brief add log handler into logger
 *
 * @param logger   logger pointer
 * @param handle   log handler
 *
 * @return
 *     - success returns 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_logger_add_handler(muggle_logger_t *logger, muggle_log_handler_t *handler);

/**
 * @brief logger write
 *
 * @param logger  logger pointer
 * @param msg     log message
 */
MUGGLE_C_EXPORT
void muggle_logger_write(muggle_logger_t *logger, muggle_log_msg_t *msg);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_LOG_LOGGER_H_ */
