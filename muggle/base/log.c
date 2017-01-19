/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif
#include "muggle/base/file.h"

#define MUGGLE_MAX_LOG_LEN 2048

enum eMuggleLogDefault
{
	MUGGLE_LOG_DEFAULT_CONSOLE = 0,
	MUGGLE_LOG_DEFAULT_FILE,
	MUGGLE_LOG_DEFAULT_WIN_DEBUG_OUT,
	MUGGLE_LOG_DEFAULT_MAX,
};

// terminal color for *nix
#define UNIX_TERMINAL_COLOR_NRM  "\x1B[0m"
#define UNIX_TERMINAL_COLOR_RED  "\x1B[31m"
#define UNIX_TERMINAL_COLOR_GRN  "\x1B[32m"
#define UNIX_TERMINAL_COLOR_YEL  "\x1B[33m"
#define UNIX_TERMINAL_COLOR_BLU  "\x1B[34m"
#define UNIX_TERMINAL_COLOR_MAG  "\x1B[35m"
#define UNIX_TERMINAL_COLOR_CYN  "\x1B[36m"
#define UNIX_TERMINAL_COLOR_WHT  "\x1B[37m"

const char* g_log_level_str[MUGGLE_LOG_LEVEL_MAX] = {
	"",
	"Info",
	"Warning",
	"Error"
};

void LogDefaultConsole(LogHandle *log_handle, LogAttribute *attr, const char *msg);
void LogDefaultFile(LogHandle *log_handle, LogAttribute *attr, const char *msg);
void LogDefaultWindowsDebugOut(LogHandle *log_handle, LogAttribute *attr, const char *msg);

LogHandle g_log_default_handles[MUGGLE_LOG_DEFAULT_MAX] = {
	{NULL, LogDefaultConsole, NULL, MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_LINE },
	{NULL, LogDefaultFile, NULL, MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE  | MUGGLE_LOG_FMT_LINE},
	{NULL, LogDefaultWindowsDebugOut, NULL, MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_LINE },
};
LogHandle* g_log_default_active[MUGGLE_LOG_DEFAULT_MAX] = {
	&g_log_default_handles[MUGGLE_LOG_DEFAULT_CONSOLE], NULL, NULL
};

int g_enable_console_color = 0;
FileHandle g_log_default_file = { 0 };
MutexHandle g_log_default_mtx[MUGGLE_LOG_DEFAULT_MAX] = { 0 };

int LogGenFmtText(LogHandle *log_handle, LogAttribute *attr, const char *msg, char *buf, int max_len)
{
	int remaining = max_len, num_write = 0;
	char *p = buf;

	if (buf == NULL || max_len <= 0)
	{
		return 0;
	}

	if (log_handle->format & MUGGLE_LOG_FMT_LEVEL)
	{
		if (attr->level > 0 && attr->level < MUGGLE_LOG_LEVEL_MAX)
		{
			num_write = snprintf(p, remaining, "|L>%s", g_log_level_str[attr->level]);
			if (num_write == -1)
			{
				return max_len;
			}
			remaining -= num_write;
			p += num_write;
		}		
	}
	if (log_handle->format & MUGGLE_LOG_FMT_FILE)
	{
		num_write = snprintf(p, remaining, "|F>%s", attr->file);
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}
	if (log_handle->format & MUGGLE_LOG_FMT_LINE)
	{
		num_write = snprintf(p, remaining, "|l>%d", attr->line);
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}
	if (log_handle->format & MUGGLE_LOG_FMT_FUNC)
	{
		num_write = snprintf(p, remaining, "|f>%s", attr->func);
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}
	if (log_handle->format & MUGGLE_LOG_FMT_DATE)
	{
		num_write = snprintf(p, remaining, "|D>%s", attr->date);
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}
	if (log_handle->format & MUGGLE_LOG_FMT_TIME)
	{
		num_write = snprintf(p, remaining, "|T>%s", attr->time);
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}
	if (msg != NULL)
	{
		num_write = snprintf(p, remaining, "|M>%s", msg);
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}

	return max_len - remaining;
}

void LogDefaultInit(const char *log_file_path, int enable_console_color)
{
	int i;
	int flags = MUGGLE_FILE_WRITE | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC;
	int attr = MUGGLE_FILE_ATTR_USER_READ | MUGGLE_FILE_ATTR_USER_WRITE | MUGGLE_FILE_ATTR_GRP_READ;
	if (log_file_path == NULL)
	{
		FileHandleOpen(&g_log_default_file, "log.txt", flags, attr);
	}
	else
	{
		FileHandleOpen(&g_log_default_file, log_file_path, flags, attr);
	}
	g_log_default_handles[MUGGLE_LOG_DEFAULT_FILE].io_target = (void*)&g_log_default_file;

	g_enable_console_color = enable_console_color;
	
	for (i = 0; i < MUGGLE_LOG_DEFAULT_MAX; ++i)
	{
		MutexInit(&g_log_default_mtx[i]);
		g_log_default_handles[i].mtx = &g_log_default_mtx[i];
		g_log_default_active[i] = &g_log_default_handles[i];
	}
}
void LogDefault(LogAttribute *attr, const char *format, ...)
{
	int i;
	char msg[MUGGLE_MAX_LOG_LEN];
	va_list args;

	va_start(args, format);	
	vsnprintf(msg, MUGGLE_MAX_LOG_LEN, format, args);
	va_end(args);

	for (i = 0; i < MUGGLE_LOG_DEFAULT_MAX; ++i)
	{
		if (g_log_default_active[i] != NULL)
		{
			g_log_default_active[i]->func(g_log_default_active[i], attr, msg);
		}
	}

#if MUGGLE_DEBUG
	if (attr->level >= MUGGLE_LOG_LEVEL_ERROR)
	{
#if MUGGLE_PLATFORM_WINDOWS
		__debugbreak();
#endif
		abort();
	}
#endif
}

void LogDefaultConsole(LogHandle *log_handle, LogAttribute *attr, const char *msg)
{
	char buf[MUGGLE_MAX_LOG_LEN + 1] = { 0 };
	FILE *fp = stdout;
	int need_color = 0;

	int write_num = LogGenFmtText(log_handle, attr, msg, buf, MUGGLE_MAX_LOG_LEN);

	if (log_handle->mtx != NULL)
	{
		MutexLock(log_handle->mtx);
	}

	if (attr->level >= MUGGLE_LOG_LEVEL_WARNING)
	{
		fp = stderr;
		need_color = 1;
	}

	if (g_enable_console_color && need_color)
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
		if (attr->level >= MUGGLE_LOG_LEVEL_ERROR)
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

	if (log_handle->mtx != NULL)
	{
		MutexUnLock(log_handle->mtx);
	}
}
void LogDefaultFile(LogHandle *log_handle, LogAttribute *attr, const char *msg)
{
	char buf[MUGGLE_MAX_LOG_LEN] = { 0 };
	int write_num = LogGenFmtText(log_handle, attr, msg, buf, MUGGLE_MAX_LOG_LEN);

	if (log_handle->mtx != NULL)
	{
		MutexLock(log_handle->mtx);
	}

	FileHandle *fh = (FileHandle*)log_handle->io_target;
	FileHandleWrite(fh, buf, (long)write_num);

	if (log_handle->mtx != NULL)
	{
		MutexUnLock(log_handle->mtx);
	}
}
void LogDefaultWindowsDebugOut(LogHandle *log_handle, LogAttribute *attr, const char *msg)
{
#if MUGGLE_PLATFORM_WINDOWS
	char buf[MUGGLE_MAX_LOG_LEN] = { 0 };
	int write_num = LogGenFmtText(log_handle, attr, msg, buf, MUGGLE_MAX_LOG_LEN);

	if (log_handle->mtx != NULL)
	{
		MutexLock(log_handle->mtx);
	}

	WCHAR w_buf[MUGGLE_MAX_LOG_LEN];
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, w_buf, MUGGLE_MAX_LOG_LEN);
	OutputDebugStringW(w_buf);

	if (log_handle->mtx != NULL)
	{
		MutexUnLock(log_handle->mtx);
	}
#endif
}