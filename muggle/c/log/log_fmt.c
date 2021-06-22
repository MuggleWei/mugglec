/******************************************************************************
 *  @file         log_fmt.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log formatter
 *****************************************************************************/
 
#include "log_fmt.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif
#include "muggle/c/os/path.h"

 // default log priority string
const char* g_muggle_log_level_str[MUGGLE_LOG_LEVEL_MAX] = {
	"",
	"TRACE",
	"INFO",
	"WARNING",
	"ERROR",
	"FATAL"
};

int muggle_log_fmt_gen(
	int fmt_flag, muggle_log_fmt_arg_t *arg,
	const char *msg, char *buf, int size)
{
	unsigned int remaining = size - 1, num_write = 0;
	char *p = buf;

	if (buf == NULL || size <= 0)
	{
		return 0;
	}

	if (remaining <= 0)
	{
		return -1;
	}

	if (fmt_flag & MUGGLE_LOG_FMT_LEVEL)
	{
		int level = arg->level >> MUGGLE_LOG_LEVEL_OFFSET;
		if (level > 0 && level < MUGGLE_LOG_LEVEL_MAX)
		{
			num_write = snprintf(p, remaining, "<L>%s|", g_muggle_log_level_str[level]);
			if (num_write < 0)
			{
				return -1;
			}
			remaining -= num_write;
			p += num_write;
		}		

		if (remaining <= 0)
		{
			return size - 1;
		}
	}
	if (fmt_flag & MUGGLE_LOG_FMT_FILE)
	{
		char filename[MUGGLE_MAX_PATH];
		muggle_path_basename(arg->file, filename, sizeof(filename));

		num_write = snprintf(p, remaining, "<F>%s:%d|", filename, arg->line);
		if (num_write == -1)
		{
			return -1;
		}
		remaining -= num_write;
		p += num_write;

		if (remaining <= 0)
		{
			return size - 1;
		}
	}
	if (fmt_flag & MUGGLE_LOG_FMT_FUNC)
	{
		num_write = snprintf(p, remaining, "<f>%s|", arg->func);
		if (num_write == -1)
		{
			return -1;
		}
		remaining -= num_write;
		p += num_write;

		if (remaining <= 0)
		{
			return size - 1;
		}
	}
	if (fmt_flag & MUGGLE_LOG_FMT_TIME)
	{
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		num_write = snprintf(p, remaining, "<T>%lld.%09d|",
			(long long)ts.tv_sec, (int)ts.tv_nsec);
		if (num_write == -1)
		{
			return -1;
		}
		remaining -= num_write;
		p += num_write;

		if (remaining <= 0)
		{
			return size - 1;
		}
	}
	if (fmt_flag & MUGGLE_LOG_FMT_THREAD)
	{
		num_write = snprintf(p, remaining, "<t>%llu|", (unsigned long long)arg->tid);
		if (num_write == -1)
		{
			return -1;
		}
		remaining -= num_write;
		p += num_write;

		if (remaining <= 0)
		{
			return size - 1;
		}
	}
	if (msg != NULL)
	{
		num_write = snprintf(p, remaining, " - %s\n", msg);
		if (num_write == -1)
		{
			return -1;
		}
		remaining -= num_write;
		p += num_write;
	}

	return size - 1 - remaining;
}
