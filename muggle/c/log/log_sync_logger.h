/******************************************************************************
 *  @file         log_sync_logger.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sync logger
 *****************************************************************************/

#ifndef MUGGLE_C_LOG_SYNC_LOGGER_H_
#define MUGGLE_C_LOG_SYNC_LOGGER_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/log/log_logger.h"

EXTERN_C_BEGIN

/**
 * @brief muggle sync logger
 */
typedef struct muggle_sync_logger
{
	muggle_logger_t logger;
}muggle_sync_logger_t;

/**
 * @brief initialize sync logger
 *
 * @param logger sync logger pointer
 *
 * @return 
 *     - success returns 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_sync_logger_init(muggle_sync_logger_t *logger);

/**
 * @brief sync logger add log handler
 *
 * @param logger   logger pointer
 * @param handler  log handler pointer
 *
 * @return 
 *     - success returns 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_sync_logger_add_handler(muggle_logger_t *logger, muggle_log_handler_t *handler);

/**
 * @brief sync logger destroy
 *
 * @param logger  logger pointer
 */
MUGGLE_C_EXPORT
void muggle_sync_logger_destroy(muggle_logger_t *logger);

/**
 * @brief prototype of logger output function
 *
 * @param logger   logger pointer
 * @param level    log level
 * @param src_loc  source location info
 * @param format   format string
 * @param ...      input arguments for format string
 */
MUGGLE_C_EXPORT
void muggle_sync_logger_log(
	struct muggle_logger *logger,
	int level,
	muggle_log_src_loc_t *src_loc,
	const char *format,
	...);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_LOG_SYNC_LOGGER_H_ */
