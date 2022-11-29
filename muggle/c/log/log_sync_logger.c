#include "log_sync_logger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "muggle/c/os/stacktrace.h"
#include "muggle/c/log/log_level.h"
#include "muggle/c/log/log_fmt.h"
#include "muggle/c/base/err.h"

int muggle_sync_logger_init(muggle_sync_logger_t *logger)
{
	memset(logger, 0, sizeof(*logger));

	muggle_logger_t *base_logger = (muggle_logger_t*)&logger->logger;
	base_logger->log = muggle_sync_logger_log;
	base_logger->add_handler = muggle_sync_logger_add_handler;
	base_logger->destroy = muggle_sync_logger_destroy;
	base_logger->lowest_log_level = MUGGLE_LOG_LEVEL_FATAL;

	return MUGGLE_OK;
}

int muggle_sync_logger_add_handler(muggle_logger_t *logger, muggle_log_handler_t *handler)
{
	size_t cnt_handler =
		sizeof(logger->handlers) / sizeof(logger->handlers[0]);
	if (logger->cnt >= (int)cnt_handler)
	{
		return MUGGLE_ERR_BEYOND_RANGE;
	}

	logger->handlers[logger->cnt++] = handler;

	if (handler->fmt)
	{
		logger->fmt_hint |= handler->fmt->fmt_hint;
	}

	if (handler->level < logger->lowest_log_level)
	{
		logger->lowest_log_level = handler->level;
	}

	return MUGGLE_OK;
}

void muggle_sync_logger_destroy(muggle_logger_t *logger)
{
	MUGGLE_UNUSED(logger);
	// do nothing
}

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
