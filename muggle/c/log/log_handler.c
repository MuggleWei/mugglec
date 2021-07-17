#include "log_handler.h"
#include <string.h>
#include "muggle/c/log/log_level.h"
#include "muggle/c/base/err.h"

void muggle_log_handler_set_fmt(muggle_log_handler_t *handler, muggle_log_fmt_t *fmt)
{
	handler->fmt = fmt;
}

void muggle_log_handler_set_level(muggle_log_handler_t *handler, int level)
{
	handler->level = level;
}

muggle_log_fmt_t* muggle_log_handler_get_fmt(muggle_log_handler_t *handler)
{
	return handler->fmt;
}

int muggle_log_handler_get_level(muggle_log_handler_t *handler)
{
	return handler->level;
}

bool muggle_log_handler_should_write(muggle_log_handler_t *handler, int level)
{
	if (level < handler->level)
	{
		return false;
	}
	return true;
}

void muggle_log_handler_set_mutex(muggle_log_handler_t *handler, bool flag)
{
	handler->need_mutex = flag;
}

int muggle_log_handler_init_default(muggle_log_handler_t *handler)
{
	memset(handler, 0, sizeof(*handler));

	muggle_log_fmt_t *fmt = muggle_log_fmt_get_simple();
	muggle_log_handler_set_fmt(handler, fmt);

	muggle_log_handler_set_level(handler, MUGGLE_LOG_LEVEL_INFO);

	handler->need_mutex = true;
	int ret = muggle_mutex_init(&handler->mtx);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	return 0;
}

int muggle_log_handler_destroy_default(muggle_log_handler_t *handler)
{
	muggle_mutex_destroy(&handler->mtx);

	return 0;
}
