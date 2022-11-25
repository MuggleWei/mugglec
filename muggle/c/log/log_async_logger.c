#include "log_async_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "muggle/c/os/stacktrace.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/log/log_level.h"
#include "muggle/c/base/err.h"
#include "muggle/c/sync/channel.h"

static muggle_thread_ret_t muggle_async_logger_run(void *arg)
{
	muggle_async_logger_t *logger = (muggle_async_logger_t*)arg;
	while (1)
	{
		muggle_log_msg_t *msg = muggle_channel_read(&logger->channel);
		if (msg == NULL)
		{
			break;
		}

		muggle_logger_write((muggle_logger_t*)logger, msg);

		free((void*)msg->payload);
		logger->p_free(msg);
	}

	return 0;
}

int muggle_async_logger_init(muggle_async_logger_t *logger, int channel_capacity)
{
	memset(logger, 0, sizeof(*logger));

	muggle_logger_t *base_logger = (muggle_logger_t*)&logger->logger;
	base_logger->log = muggle_async_logger_log;
	base_logger->add_handler = muggle_async_logger_add_handler;
	base_logger->destroy = muggle_async_logger_destroy;
	base_logger->lowest_log_level = MUGGLE_LOG_LEVEL_FATAL;

	logger->p_alloc = malloc;
	logger->p_free = free;
	int ret = muggle_channel_init(&logger->channel, channel_capacity, 0);
	if (ret != 0)
	{
		fprintf(stderr, "failed initialize async logger channel\n");
		return ret;
	}

	ret = muggle_thread_create(&logger->thread, muggle_async_logger_run, logger);
	if (ret != 0)
	{
		fprintf(stderr, "failed initialize async logger thread\n");
		return ret;
	}

	return 0;
}

int muggle_async_logger_add_handler(muggle_logger_t *logger, muggle_log_handler_t *handler)
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

	// handler in async logger don't need mutex
	handler->need_mutex = false;

	return MUGGLE_OK;
}

void muggle_async_logger_destroy(muggle_logger_t *logger)
{
	muggle_async_logger_t *async_logger = (muggle_async_logger_t*)logger;

	muggle_channel_write(&async_logger->channel, NULL);
	muggle_thread_join(&async_logger->thread);

	muggle_channel_destroy(&async_logger->channel);
}

void muggle_async_logger_log(
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

	muggle_async_logger_t *async_logger = (muggle_async_logger_t*)logger;
	muggle_log_msg_t *msg = (muggle_log_msg_t*)async_logger->p_alloc(sizeof(muggle_log_msg_t));
	if (msg == NULL)
	{
		return;
	}

	// level
	msg->level = level;

	// timestamp
	if (logger->fmt_hint & MUGGLE_LOG_FMT_TIME)
	{
		timespec_get(&msg->ts, TIME_UTC);
	}

	// thread id
	if (logger->fmt_hint & MUGGLE_LOG_FMT_THREAD)
	{
		msg->tid = muggle_thread_current_id();
	}

	// source location
	memcpy(&msg->src_loc, src_loc, sizeof(msg->src_loc));

	// payload
	char *payload = (char*)malloc(MUGGLE_LOG_MSG_MAX_LEN);
	va_list args;

	va_start(args, format);
	vsnprintf(payload, MUGGLE_LOG_MSG_MAX_LEN, format, args);
	va_end(args);

	msg->payload = payload;

	// write
	muggle_channel_write(&async_logger->channel, msg);

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
