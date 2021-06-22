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
#include "muggle/c/base/thread.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_LOG_LEVEL_OFFSET = 8,
	MUGGLE_LOG_LEVEL_TRACE = 1 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_INFO = 2 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_WARNING = 3 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_ERROR = 4 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_FATAL = 5 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_MAX = 6,
};

enum
{
	MUGGLE_LOG_FMT_LEVEL = 0x01,  //!< show log level in log
	MUGGLE_LOG_FMT_FILE = 0x02,   //!< show file name in log
	MUGGLE_LOG_FMT_FUNC = 0x04,   //!< show function name in log
	MUGGLE_LOG_FMT_TIME = 0x08,   //!< show print time in log
	MUGGLE_LOG_FMT_THREAD = 0x10, //!< show thread id in log
};

typedef struct muggle_log_fmt_arg_tag
{
	int              level; //!< log level
	unsigned int     line;  //!< line in file
	const char       *file; //!< file name
	const char       *func; //!< function name
	muggle_thread_id tid;   //!< thread id
}muggle_log_fmt_arg_t;

/**
 * @brief generate formated message
 *
 * @param fmt_flag format flag
 * @param arg      format arguments
 * @param msg      original message
 * @param buf      the formated message output buffer
 * @param size     the size of buf
 *
 * @return  the len of formated message, negative represent failed
 */
MUGGLE_C_EXPORT
int muggle_log_fmt_gen(
	int fmt_flag, muggle_log_fmt_arg_t *arg,
	const char *msg, char *buf, int size);

EXTERN_C_END

#endif
