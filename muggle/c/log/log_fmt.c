/******************************************************************************
 *  @file         log_fmt.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log formatter
 *****************************************************************************/
 
#include "log_fmt.h"
#include <stdio.h>
#include <string.h>
#include "muggle/c/os/path.h"
#include "log_level.h"
#if MUGGLE_PLATFORM_WINDOWS
#include "muggle/c/time/win_gmtime.h"
#endif

/**
 * @brief default log format function
 *
 * @param msg      log message
 * @param buf      the formated message output buffer
 * @param bufsize  the size of buf
 *
 * @return the len of formated message, negative represent failed
 */
static int muggle_log_fmt_simple(const muggle_log_msg_t *msg, char *buf, size_t bufsize)
{
	const char *level = muggle_log_level_to_str(msg->level);

	char filename[MUGGLE_MAX_PATH];
	muggle_path_basename(msg->src_loc.file, filename, sizeof(filename));

	const char *payload = "";
	if (msg->payload)
	{
		payload = msg->payload;
	}

	return (int)snprintf(buf, bufsize,
		"%s|%s:%u - %s\n",
		level,
		filename, (unsigned int)msg->src_loc.line,
		payload);
}

/**
 * @brief default log format with time represent YYYY-MM-DDThh:mm:ss.sss
 *
 * @param msg      log message
 * @param buf      the formated message output buffer
 * @param bufsize  the size of buf
 *
 * @return the len of formated message, negative represent failed
 */
static int muggle_log_fmt_complicated(const muggle_log_msg_t *msg, char *buf, size_t bufsize)
{
	const char *level = muggle_log_level_to_str(msg->level);

	char filename[MUGGLE_MAX_PATH];
	muggle_path_basename(msg->src_loc.file, filename, sizeof(filename));

	struct tm t;
	gmtime_r(&msg->ts.tv_sec, &t);

	const char *payload = "";
	if (msg->payload)
	{
		payload = msg->payload;
	}

	return (int)snprintf(buf, bufsize,
		"%s|%d-%02d-%02dT%02d:%02d:%02d.%03d|%s:%u|%s|%llu - %s\n",
		level,
		(int)t.tm_year+1900, (int)t.tm_mon+1, (int)t.tm_mday,
		(int)t.tm_hour, (int)t.tm_min, (int)t.tm_sec,
		(int)msg->ts.tv_nsec / 1000000,
		filename, (unsigned int)msg->src_loc.line,
		msg->src_loc.func,
		(unsigned long long)msg->tid,
		payload);
}

void init_fmt(muggle_log_fmt_t *p_fmt, int hint, func_muggle_log_fmt func)
{
	memset(p_fmt, 0, sizeof(*p_fmt));
	p_fmt->fmt_hint = hint;
	p_fmt->fmt_func = func;
}

muggle_log_fmt_t* muggle_log_fmt_get_simple()
{
	static muggle_log_fmt_t fmt = {
		MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_FUNC,
		muggle_log_fmt_simple
	};
	return &fmt;
}

muggle_log_fmt_t* muggle_log_fmt_get_complicated()
{
	static muggle_log_fmt_t fmt = {
		MUGGLE_LOG_FMT_ALL,
		muggle_log_fmt_complicated
	};
	return &fmt;
}
