/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "log_handle_console.h"
#include <stdio.h>
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
	muggle_atomic_int async_capacity,
	int enable_color)
{
	handle->type = MUGGLE_LOG_TYPE_CONSOLE;
	if (write_type >= MUGGLE_LOG_WRITE_TYPE_MAX || write_type < 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}
	handle->write_type = write_type;
	handle->fmt_flag = fmt_flag;
	handle->enable_color = enable_color;
	async_capacity = async_capacity <= 8 ? 1024 * 8 : async_capacity;

	switch (write_type)
	{
		case MUGGLE_LOG_WRITE_TYPE_SYNC:
		{
			muggle_mutex_init(&handle->mutex);
		}break;
		case MUGGLE_LOG_WRITE_TYPE_ASYNC:
		{
			muggle_ringbuffer_init(
				&handle->ring,
				async_capacity,
				MUGGLE_RINGBUFFER_FLAG_WRITE_LOCK | MUGGLE_RINGBUFFER_FLAG_SINGLE_READER); 
			muggle_thread_create(&handle->thread, muggle_log_handle_run_async, handle);
		}break;
	}

	return MUGGLE_OK;
}

int muggle_log_handle_console_destroy(muggle_log_handle_t *handle)
{
	switch (handle->write_type)
	{
		case MUGGLE_LOG_WRITE_TYPE_SYNC:
		{
			muggle_mutex_destroy(&handle->mutex);
		}break;
		case MUGGLE_LOG_WRITE_TYPE_ASYNC:
		{
			muggle_ringbuffer_write(&handle->ring, NULL);
			muggle_thread_join(&handle->thread);
		}break;
	}

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
	if (ret < 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	FILE *fp = stdout;
	if (arg->level >= MUGGLE_LOG_LEVEL_WARNING)
	{
		fp = stderr;
	}

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_lock(&handle->mutex);
	}

	if (handle->enable_color && arg->level >= MUGGLE_LOG_LEVEL_WARNING)
	{
#if MUGGLE_PLATFORM_WINDOWS
		const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

		// get the current text color
		CONSOLE_SCREEN_BUFFER_INFO sb_info;
		GetConsoleScreenBufferInfo(stdout_handle, &sb_info);
		const WORD old_sb_attrs = sb_info.wAttributes;

		// change text color
		fflush(stdout);
		if (attr->level >= MUGGLE_LOG_LEVEL_ERROR)
		{
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
		}
		else if (attr->level >= MUGGLE_LOG_LEVEL_WARNING)
		{
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		}
		
		fprintf(fp, buf);

		fflush(stdout);

		// restores text color
		SetConsoleTextAttribute(stdout_handle, old_sb_attrs);
#else
		const char *color_str = UNIX_TERMINAL_COLOR_YEL;
		if (arg->level >= MUGGLE_LOG_LEVEL_ERROR)
		{
			color_str = UNIX_TERMINAL_COLOR_RED;
		}
		fprintf(fp, "%s%s", color_str, buf);
		fprintf(fp, "\033[m");
#endif
	}
	else
	{
		fprintf(fp, "%s", buf);
	}

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_unlock(&handle->mutex);
	}

	return MUGGLE_OK;
}
