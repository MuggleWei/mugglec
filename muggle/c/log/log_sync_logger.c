#include "log_sync_logger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "muggle/c/os/stacktrace.h"
#include "log_level.h"
#include "log_fmt.h"

void muggle_sync_logger_log(
	struct muggle_logger *logger,
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
	if (level >= MUGGLE_LOG_LEVEL_FATAL)
	{
		muggle_print_stacktrace();
#if MUGGLE_PLATFORM_WINDOWS
		__debugbreak();
#endif
		abort();
	}
#endif
}
