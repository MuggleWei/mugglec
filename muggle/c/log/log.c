/******************************************************************************
 *  @file         log.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log
 *****************************************************************************/
 
#include "log.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/log/log_logger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#if MUGGLE_DEBUG
#include "muggle/c/os/stacktrace.h"
#endif
#include "muggle/c/log/log_level.h"
#include "muggle/c/log/log_fmt.h"
#include "muggle/c/log/log_handler.h"
#include "muggle/c/log/log_console_handler.h"
#include "muggle/c/log/log_file_rotate_handler.h"
#include "muggle/c/os/path.h"
#include "muggle/c/os/os.h"

static int muggle_log_simple_init_fmt(const muggle_log_msg_t *msg, char *buf, size_t bufsize)
{
	const char *level = muggle_log_level_to_str(msg->level);

	char filename[MUGGLE_MAX_PATH];
	muggle_path_basename(msg->src_loc.file, filename, sizeof(filename));

	const char *payload = "";
	if (msg->payload)
	{
		payload = msg->payload;
	}

	return (int)snprintf(buf, bufsize,
		"%s|%llu.%09d|%s:%u|%s|%llu - %s\n",
		level,
		(unsigned long long)msg->ts.tv_sec,
		(int)msg->ts.tv_nsec / 1000,
		filename,
		(unsigned int)msg->src_loc.line,
		msg->src_loc.func,
		(unsigned long long)msg->tid,
		payload);
}

static int muggle_log_simple_init_log_path(char *log_path, size_t log_path_size)
{
	char process_path[MUGGLE_MAX_PATH];
	if (muggle_os_process_path(process_path, sizeof(process_path)) != 0)
	{
		fprintf(stderr, "failed get process path");
		return -1;
	}

    char process_name[MUGGLE_MAX_PATH];
    if (muggle_path_basename(process_path, process_name, sizeof(process_name)) != 0)
    {
		fprintf(stderr, "failed get basename from path: %s", process_path);
        return -1;
    }

	char log_name[MUGGLE_MAX_PATH];
	snprintf(log_name, sizeof(log_name), "%s.log", process_name);

	if (muggle_path_join("log", log_name, log_path, log_path_size) != 0)
	{
		fprintf(stderr, "failed join log path");
		return -1;
	}

	return 0;
}

int muggle_log_simple_init(int level_console, int level_file_rotating)
{
	static muggle_log_fmt_t formatter = {
		MUGGLE_LOG_FMT_ALL,
		muggle_log_simple_init_fmt
	};
	muggle_logger_t *logger = muggle_logger_default();

	// console handler
	static muggle_log_console_handler_t console_handler;
	muggle_log_console_handler_init(&console_handler, 1);
	muggle_log_handler_set_level((muggle_log_handler_t*)&console_handler, level_console);
	muggle_log_handler_set_fmt((muggle_log_handler_t*)&console_handler, &formatter); 

	// file rotate handler
	char log_path[MUGGLE_MAX_PATH];
	if (muggle_log_simple_init_log_path(log_path, sizeof(log_path)) != 0)
	{
		return -1;
	}

	static muggle_log_file_rotate_handler_t rot_file_handler;
	int ret = muggle_log_file_rotate_handler_init(
		&rot_file_handler, log_path, 1024 * 1024 * 64, 5);
	if (ret != 0)
	{
		fprintf(stderr, "failed init file rotate handler with path: %s", log_path);
		return -1;
	}

	// add handler into logger
	muggle_logger_add_handler(logger, (muggle_log_handler_t*)&console_handler);
	muggle_logger_add_handler(logger, (muggle_log_handler_t*)&rot_file_handler);

	return 0;
}
