/******************************************************************************
 *  @file         log_async_logger.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-18
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec async logger
 *****************************************************************************/

#ifndef MUGGLE_C_LOG_ASYNC_LOGGER_H_
#define MUGGLE_C_LOG_ASYNC_LOGGER_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/log/log_logger.h"
#include "muggle/c/sync/channel.h"

EXTERN_C_BEGIN

typedef void* (*muggle_log_async_logger_alloc)(size_t size);
typedef void (*muggle_log_async_logger_free)(void *ptr);

/**
 * @brief muggle async logger
 */
typedef struct muggle_async_logger
{
	muggle_logger_t logger;
	muggle_log_async_logger_alloc p_alloc;
	muggle_log_async_logger_free p_free;
	muggle_channel_t channel;
	muggle_thread_t thread;
}muggle_async_logger_t;

/**
 * @brief initialize async logger
 *
 * @param logger            async logger pointer
 * @param cahnnel_capacity  async logger's channel capacity
 *
 * @return 
 *     - success returns 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_async_logger_init(muggle_async_logger_t *logger, int channel_capacity);

/**
 * @brief async logger add log handler
 *
 * @param logger   logger pointer
 * @param handler  log handler pointer
 *
 * @return 
 *     - success returns 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_async_logger_add_handler(muggle_logger_t *logger, muggle_log_handler_t *handler);

/**
 * @brief async logger destroy
 *
 * @param logger  logger pointer
 */
MUGGLE_C_EXPORT
void muggle_async_logger_destroy(muggle_logger_t *logger);

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
void muggle_async_logger_log(
	struct muggle_logger *logger,
	int level,
	muggle_log_src_loc_t *src_loc,
	const char *format,
	...);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_LOG_ASYNC_LOGGER_H_ */
