#include "log_handler_console.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "muggle/c/base/err.h"
#include "muggle/c/sync/mutex.h"
#include "log_level.h"

 // terminal color for *nix
#define UNIX_TERMINAL_COLOR_NRM  "\x1B[0m"
#define UNIX_TERMINAL_COLOR_RED  "\x1B[31m"
#define UNIX_TERMINAL_COLOR_GRN  "\x1B[32m"
#define UNIX_TERMINAL_COLOR_YEL  "\x1B[33m"
#define UNIX_TERMINAL_COLOR_BLU  "\x1B[34m"
#define UNIX_TERMINAL_COLOR_MAG  "\x1B[35m"
#define UNIX_TERMINAL_COLOR_CYN  "\x1B[36m"
#define UNIX_TERMINAL_COLOR_WHT  "\x1B[37m"

/**
 * @brief write log
 *
 * @param handler  log handler pointer
 * @param msg      muggle log msg
 *
 * @return
 *     - on success, return number of bytes be writed
 *     - otherwise return negative number
 */
static int muggle_log_handler_console_write(
	struct muggle_log_handler *base_handler, const muggle_log_msg_t *msg)
{
	char buf[MUGGLE_LOG_MSG_MAX_LEN];
	muggle_log_fmt_t *fmt = muggle_log_handler_get_fmt(base_handler);
	if (fmt == NULL)
	{
		return -1;
	}

	int ret = fmt->fmt_func(msg, buf, sizeof(buf));
	if (ret < 0)
	{
		return -2;
	}

	muggle_log_handler_console_t *handler = (muggle_log_handler_console_t*)base_handler;

	FILE *fp = stdout;
	if (msg->level >= MUGGLE_LOG_LEVEL_WARNING)
	{
		fp = stderr;
	}

	if (handler->handler.need_mutex)
	{
		muggle_mutex_lock(&handler->mtx);
	}

	if (handler->enable_color && msg->level >= MUGGLE_LOG_LEVEL_WARNING)
	{
#if MUGGLE_PLATFORM_WINDOWS
		const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

		// get the current text color
		CONSOLE_SCREEN_BUFFER_INFO sb_info;
		GetConsoleScreenBufferInfo(stdout_handle, &sb_info);
		const WORD old_sb_attrs = sb_info.wAttributes;

		// change text color
		fflush(fp);

		if (arg->level >= MUGGLE_LOG_LEVEL_ERROR)
		{
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
		}
		else if (arg->level >= MUGGLE_LOG_LEVEL_WARNING)
		{
			SetConsoleTextAttribute(stdout_handle,
				FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		}
		
		ret = (int)fwrite(buf, 1, ret, fp);

		fflush(fp);

		// restores text color
		SetConsoleTextAttribute(stdout_handle, old_sb_attrs);
#else
		const char *color_str = UNIX_TERMINAL_COLOR_YEL;
		if (msg->level >= MUGGLE_LOG_LEVEL_ERROR)
		{
			fwrite(UNIX_TERMINAL_COLOR_RED, 1, strlen(UNIX_TERMINAL_COLOR_RED), fp);
		}
		else
		{
			fwrite(UNIX_TERMINAL_COLOR_YEL, 1, strlen(UNIX_TERMINAL_COLOR_YEL), fp);
		}
		ret = (int)fwrite(buf, 1, ret, fp);
		fwrite("\033[m", 1, strlen("\033[m"), fp);

		fflush(fp);
#endif
	}
	else
	{
		ret = (int)fwrite(buf, 1, ret, fp);
		fflush(fp);
	}

	if (handler->handler.need_mutex)
	{
		muggle_mutex_unlock(&handler->mtx);
	}

	return ret;
}

/**
 * @brief destroy console log handler
 *
 * @param handler  log handler pointer
 *
 * @return
 *     - success returns 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
static int muggle_log_handler_console_destroy(
	muggle_log_handler_t *base_handler)
{
	muggle_log_handler_console_t *handler = (muggle_log_handler_console_t*)base_handler;
	muggle_mutex_destroy(&handler->mtx);

	return MUGGLE_OK;
}

int muggle_log_handler_console_init(
	muggle_log_handler_console_t *handler,
	int enable_color)
{
	muggle_log_handler_init_default((muggle_log_handler_t*)handler);
	handler->handler.write = muggle_log_handler_console_write;
	handler->handler.destroy = muggle_log_handler_console_destroy;

	int ret = muggle_mutex_init(&handler->mtx);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}
	handler->enable_color = enable_color;

	return MUGGLE_OK;
}
