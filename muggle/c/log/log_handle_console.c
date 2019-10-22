/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "log_handle_console.h"
#include <stdio.h>
#include <string.h>
#include "muggle/c/base/err.h"

 // terminal color for *nix
#define UNIX_TERMINAL_COLOR_NRM  "\x1B[0m"
#define UNIX_TERMINAL_COLOR_RED  "\x1B[31m"
#define UNIX_TERMINAL_COLOR_GRN  "\x1B[32m"
#define UNIX_TERMINAL_COLOR_YEL  "\x1B[33m"
#define UNIX_TERMINAL_COLOR_BLU  "\x1B[34m"
#define UNIX_TERMINAL_COLOR_MAG  "\x1B[35m"
#define UNIX_TERMINAL_COLOR_CYN  "\x1B[36m"
#define UNIX_TERMINAL_COLOR_WHT  "\x1B[37m"

int muggle_log_handle_console_init(
	muggle_log_handle_t *handle,
	int write_type,
	int fmt_flag,
	int level,
	muggle_atomic_int async_capacity,
	muggle_log_handle_async_alloc p_alloc,
	muggle_log_handle_async_free p_free,
	int enable_color)
{
	handle->type = MUGGLE_LOG_TYPE_CONSOLE;
	int ret = muggle_log_handle_base_init(handle, write_type, fmt_flag,	level, async_capacity, p_alloc, p_free);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}
	handle->console.enable_color = enable_color;

	return MUGGLE_OK;
}

int muggle_log_handle_console_destroy(muggle_log_handle_t *handle)
{
	return MUGGLE_OK;
}

int muggle_log_handle_console_output(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
)
{
	int ret;
	char buf[MUGGLE_LOG_MAX_LEN] = { 0 };

	ret = muggle_log_fmt_gen(handle->fmt_flag, arg, msg, buf, sizeof(buf));
	if (ret <= 0)
	{
		return ret;
	}

	FILE *fp = stdout;
	if (arg->level >= MUGGLE_LOG_LEVEL_WARNING)
	{
		fp = stderr;
	}

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_lock(&handle->sync.mutex);
	}

	if (handle->console.enable_color && arg->level >= MUGGLE_LOG_LEVEL_WARNING)
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
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		}
		
		ret = (int)fwrite(buf, 1, ret, fp);

		fflush(fp);

		// restores text color
		SetConsoleTextAttribute(stdout_handle, old_sb_attrs);
#else
		const char *color_str = UNIX_TERMINAL_COLOR_YEL;
		if (arg->level >= MUGGLE_LOG_LEVEL_ERROR)
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

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_unlock(&handle->sync.mutex);
	}

	return ret;
}
