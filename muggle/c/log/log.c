#include "log.h"
#include <stdlib.h>
#include <stdarg.h>

muggle_log_category_t g_log_default_category = {
	{NULL}, 0, MUGGLE_LOG_LEVEL_FATAL + 1
};

void muggle_log_add_handle(muggle_log_handle_t *handle)
{
	muggle_log_category_add(&g_log_default_category, handle);
}

void muggle_log_destroy()
{
	muggle_log_category_destroy(&g_log_default_category, 1);
}

void muggle_log_function(
	muggle_log_category_t *category,
	muggle_log_fmt_arg_t *arg,
	const char *format,
	...)
{
	if (category->lowest_log_level > arg->level)
	{
		return;
	}

	char msg[MUGGLE_LOG_MAX_LEN];
	va_list args;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	muggle_log_category_write(category, arg, msg);

#if MUGGLE_DEBUG
	if (arg->level >= MUGGLE_LOG_LEVEL_FATAL)
	{
#if MUGGLE_PLATFORM_WINDOWS
		__debugbreak();
#endif
		abort();
	}
#endif
}
