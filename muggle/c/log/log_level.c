/******************************************************************************
 *  @file         log_level.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log level
 *****************************************************************************/

#include "log_level.h"

const char* muggle_log_level_to_str(int log_level)
{
	static const char* s_str_log_level_unknown = "UNKNOWN";
	static const char* s_str_log_level[MUGGLE_LOG_LEVEL_MAX] = {
		"TRACE",
		"DEBUG",
		"INFO",
		"WARNING",
		"ERROR",
		"FATAL"
	};
	int level = log_level >> MUGGLE_LOG_LEVEL_OFFSET;
	if (level >= 0 && level < MUGGLE_LOG_LEVEL_MAX)
	{
		return s_str_log_level[level];
	}
	return s_str_log_level_unknown;
}
