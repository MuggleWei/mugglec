#include "log_console_handler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "muggle/c/base/err.h"
#include "muggle/c/sync/mutex.h"
#include "log_level.h"

/*
 * unix terminal colors
 *
 * output colored text, need print or echo -e the control characters for 
 * required color, then output text, and then reset the output back to default.
 *
 * "\x1B[${code}m" or "\033[${code}m", ${code} represent color code
 *
 * reset codes: 0
 *
 * color codes:
 * | color        | foreground | background |
 * | ----         | ----       | ----       |
 * | default      | 39         | 49         |
 * | black        | 30         | 40         |
 * | dark red     | 31         | 41         |
 * | dark green   | 32         | 42         |
 * | dark yellow  | 33         | 43         |
 * | dark blue    | 34         | 44         |
 * | dark magenta | 35         | 45         |
 * | dark cyan    | 36         | 46         |
 * | light gray   | 37         | 47         |
 * | dark gray    | 90         | 100        |
 * | red          | 91         | 101        |
 * | green        | 92         | 102        |
 * | orange       | 93         | 103        |
 * | blue         | 94         | 104        |
 * | magenta      | 95         | 105        |
 * | cyan         | 96         | 106        |
 * | white        | 97         | 107        |
 *
 * format for foreground color is:
 * "\x1B[" + "<0 or 1, meaning nromal or bold>;" + "<color code>" + "m"
 *
 * format for background:
 * "\x1B[" + "<color code>" + "m"
 * */

#define UNIX_TERMINAL_COLOR(code) "\x1B[" #code "m"

 // terminal color for *nix
#define UNIX_TERMINAL_COLOR_RST UNIX_TERMINAL_COLOR(0)
#define UNIX_TERMINAL_COLOR_RED UNIX_TERMINAL_COLOR(31)
#define UNIX_TERMINAL_COLOR_GRN UNIX_TERMINAL_COLOR(32)
#define UNIX_TERMINAL_COLOR_YEL UNIX_TERMINAL_COLOR(33)
#define UNIX_TERMINAL_COLOR_BLU UNIX_TERMINAL_COLOR(34)
#define UNIX_TERMINAL_COLOR_MAG UNIX_TERMINAL_COLOR(35)
#define UNIX_TERMINAL_COLOR_CYN UNIX_TERMINAL_COLOR(36)
#define UNIX_TERMINAL_COLOR_WHT UNIX_TERMINAL_COLOR(37)

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
static int muggle_log_console_handler_write(
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

	FILE *fp = stdout;
	if (msg->level >= MUGGLE_LOG_LEVEL_WARNING)
	{
		fp = stderr;
	}

	muggle_log_console_handler_t *handler = (muggle_log_console_handler_t*)base_handler;

	if (base_handler->need_mutex)
	{
		muggle_mutex_lock(&base_handler->mtx);
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

		if (msg->level >= MUGGLE_LOG_LEVEL_ERROR)
		{
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
		}
		else if (msg->level >= MUGGLE_LOG_LEVEL_WARNING)
		{
			SetConsoleTextAttribute(stdout_handle,
				FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		}
		
		ret = (int)fwrite(buf, 1, ret, fp);

		fflush(fp);

		// restores text color
		SetConsoleTextAttribute(stdout_handle, old_sb_attrs);
#else
		if (msg->level >= MUGGLE_LOG_LEVEL_ERROR)
		{
			fwrite(UNIX_TERMINAL_COLOR_RED, 1, strlen(UNIX_TERMINAL_COLOR_RED), fp);
		}
		else
		{
			fwrite(UNIX_TERMINAL_COLOR_YEL, 1, strlen(UNIX_TERMINAL_COLOR_YEL), fp);
		}
		ret = (int)fwrite(buf, 1, ret, fp);
		fwrite(UNIX_TERMINAL_COLOR_RST, 1, strlen(UNIX_TERMINAL_COLOR_RST), fp);

		fflush(fp);
#endif
	}
	else
	{
		ret = (int)fwrite(buf, 1, ret, fp);
		fflush(fp);
	}

	if (base_handler->need_mutex)
	{
		muggle_mutex_unlock(&base_handler->mtx);
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
static int muggle_log_console_handler_destroy(
	muggle_log_handler_t *base_handler)
{
	return muggle_log_handler_destroy_default(base_handler);
}

int muggle_log_console_handler_init(
	muggle_log_console_handler_t *handler,
	int enable_color)
{
	memset(handler, 0, sizeof(*handler));

	int ret = muggle_log_handler_init_default((muggle_log_handler_t*)handler);
	if (ret != 0)
	{
		return ret;
	}

	handler->handler.write = muggle_log_console_handler_write;
	handler->handler.destroy = muggle_log_console_handler_destroy;

	handler->enable_color = enable_color;

	return MUGGLE_OK;
}
