/******************************************************************************
 *  @file         log_handler_consoler.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec console log handler
 *****************************************************************************/

#ifndef MUGGLE_C_LOG_HANDLER_CONSOLE_H_
#define MUGGLE_C_LOG_HANDLER_CONSOLE_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/sync/mutex.h"
#include "muggle/c/log/log_handler.h"
#include "muggle/c/log/log_handler.h"

EXTERN_C_BEGIN

/**
 * @brief muggle console log handler
 */
typedef struct muggle_log_handler_console
{
	muggle_log_handler_t handler;      //!< base log handler
	muggle_mutex_t       mtx;          //!< mutex
	int                  enable_color; //!< enable color
}muggle_log_handler_console_t;

/**
 * @brief initialize a console log handle
 *
 * @param handler       console log handler pointer
 * @param enable_color  enable console log handler color
 *
 * @return 
 */
MUGGLE_C_EXPORT
int muggle_log_handler_console_init(
	muggle_log_handler_console_t *handler,
	int enable_color);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_LOG_HANDLER_CONSOLE_H_ */
