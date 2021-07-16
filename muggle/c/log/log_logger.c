#include "log_logger.h"
#include "muggle/c/base/err.h"
#include "muggle/c/log/log_level.h"
#include "muggle/c/log/log_handler.h"
#include "muggle/c/log/log_sync_logger.h"

int muggle_logger_add_handler(muggle_logger_t *logger, muggle_log_handler_t *handler)
{
	if (logger->cnt >= (sizeof(logger->handlers) / sizeof(logger->handlers[0])))
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

void muggle_logger_write(muggle_logger_t *logger, muggle_log_msg_t *msg)
{
	for (int i = 0; i < logger->cnt; i++)
	{
		muggle_log_handler_t *handler = logger->handlers[i];
		if (muggle_log_handler_should_write(handler, msg->level))
		{
			handler->write(handler, msg);
		}
	}
}

muggle_logger_t* muggle_logger_default()
{
	static muggle_sync_logger_t logger =
	{
		{
			muggle_sync_logger_log,
			{NULL}, 0, 0, MUGGLE_LOG_LEVEL_FATAL
		}
	};
	return (muggle_logger_t*)&logger;
}

