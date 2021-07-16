/******************************************************************************
 *  @file         log.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log
 *****************************************************************************/
 
#include "log.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/log/log_logger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#if MUGGLE_DEBUG
#include "muggle/c/os/stacktrace.h"
#endif
#include "muggle/c/log/log_level.h"
#include "muggle/c/log/log_fmt.h"
#include "muggle/c/log/log_handler.h"
#include "muggle/c/log/log_handler_console.h"

muggle_logger_t* muggle_logger_default()
{
	static muggle_logger_t logger =
	{
		{NULL}, 0, 0, MUGGLE_LOG_LEVEL_FATAL
	};
	return &logger;
}

void muggle_log_function(
	muggle_logger_t *logger,
	int level,
	muggle_log_src_loc_t *src_loc,
	const char *format,
	...)
{
	if (logger->lowest_log_level > level)
	{
		return;
	}

	muggle_log_msg_t msg;
	memset(&msg, 0, sizeof(msg));

	// level
	msg.level = level;

	// timestamp
	if (logger->fmt_hint & MUGGLE_LOG_FMT_TIME)
	{
		timespec_get(&msg.ts, TIME_UTC);
	}

	// thread id
	if (logger->fmt_hint & MUGGLE_LOG_FMT_THREAD)
	{
		msg.tid = muggle_thread_current_id();
	}

	// source location
	memcpy(&msg.src_loc, src_loc, sizeof(msg.src_loc));

	// payload
	char payload[MUGGLE_LOG_MSG_MAX_LEN];
	va_list args;

	va_start(args, format);
	vsnprintf(payload, sizeof(payload), format, args);
	va_end(args);

	msg.payload = payload;

	// write
	muggle_logger_write(logger, &msg);

#if MUGGLE_DEBUG
	if (arg->level >= MUGGLE_LOG_LEVEL_FATAL)
	{
		muggle_print_stacktrace();
#if MUGGLE_PLATFORM_WINDOWS
		__debugbreak();
#endif
		abort();
	}
#endif
}

int muggle_log_simple_init(int level_console, int level_file_rotating)
{
	muggle_logger_t *logger = muggle_logger_default();

	// console handler
	static muggle_log_handler_console_t console_handler;
	muggle_log_handler_console_init(&console_handler, 1);
	muggle_log_handler_set_level((muggle_log_handler_t*)&console_handler, level_console);

	muggle_logger_add_handler(logger, (muggle_log_handler_t*)&console_handler);

	return 0;
}
