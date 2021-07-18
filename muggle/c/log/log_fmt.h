/******************************************************************************
 *  @file         log_fmt.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log formatter
 *****************************************************************************/
 
#ifndef MUGGLE_C_LOG_FMT_H_
#define MUGGLE_C_LOG_FMT_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/log/log_msg.h"

EXTERN_C_BEGIN


/**
 * @brief log formatter arguments
 */
enum
{
	MUGGLE_LOG_FMT_LEVEL  = 1 << 0,  //!< show log level in log
	MUGGLE_LOG_FMT_FILE   = 1 << 1,  //!< show file name in log
	MUGGLE_LOG_FMT_FUNC   = 1 << 2,  //!< show function name in log
	MUGGLE_LOG_FMT_TIME   = 1 << 3,  //!< show print time in log
	MUGGLE_LOG_FMT_THREAD = 1 << 4,  //!< show thread id in log

	MUGGLE_LOG_FMT_ALL =
		MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE |
		MUGGLE_LOG_FMT_FUNC | MUGGLE_LOG_FMT_TIME |
		MUGGLE_LOG_FMT_THREAD,
};

/**
 * @brief prototype of log format function
 *
 * @param msg      log message
 * @param buf      the formated message output buffer
 * @param bufsize  the size of buf
 *
 * @return the len of formated message, negative represent failed
 */
typedef int (*func_muggle_log_fmt)(const muggle_log_msg_t *msg, char *buf, size_t bufsize);

typedef struct muggle_log_fmt
{
	int fmt_hint;                  //!< formatter hint, bitwise or of MUGGLE_LOG_FMT_*
	func_muggle_log_fmt fmt_func;  //!< formatter callback function
}muggle_log_fmt_t;

/**
 * @brief initialize muggle log formatter
 *
 * @param p_fmt  muggle log formatter pointer
 * @param hint   formatter hint 
 * @param func   formatter callback function
 */
MUGGLE_C_EXPORT
void init_fmt(muggle_log_fmt_t *p_fmt, int hint, func_muggle_log_fmt func);

/**
 * @brief get default muggle log formatter
 *
 * @return muggle log formatter
 */
MUGGLE_C_EXPORT
muggle_log_fmt_t* muggle_log_fmt_get_simple();

/**
 * @brief get muggle log formatter with level, loc info, iso8601 timestamp and thread id
 *
 * @return muggle log formatter
 */
MUGGLE_C_EXPORT
muggle_log_fmt_t* muggle_log_fmt_get_complicated();

EXTERN_C_END

#endif
