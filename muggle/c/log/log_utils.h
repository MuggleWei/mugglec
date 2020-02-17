#ifndef MUGGLE_C_LOG_UTILS_H_
#define MUGGLE_C_LOG_UTILS_H_

#include "muggle/c/log/log.h"

EXTERN_C_BEGIN

/*
 * simple initialize log with console and file_rotate category
 * @level_console: console output level
 * @level_file_rotating: file rotating output level
 * */
int muggle_log_simple_init(int level_console, int level_file_rotating);

EXTERN_C_END

#endif
