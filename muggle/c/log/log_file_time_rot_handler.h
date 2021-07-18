/******************************************************************************
 *  @file         log_file_time_rot_handler.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec file time rotate handler
 *****************************************************************************/

#ifndef MUGGLE_C_LOG_FILE_TIME_ROTATE_HANDLER_H_
#define MUGGLE_C_LOG_FILE_TIME_ROTATE_HANDLER_H_

#include "muggle/c/base/macro.h"
#include <stdio.h>
#include "muggle/c/log/log_handler.h"

EXTERN_C_BEGIN

#define MUGGLE_LOG_TIME_ROTATE_UNIT_SEC  's' //!< rotate unit - second
#define MUGGLE_LOG_TIME_ROTATE_UNIT_MIN  'm' //!< rotate unit - minute
#define MUGGLE_LOG_TIME_ROTATE_UNIT_HOUR 'h' //!< rotate unit - hour
#define MUGGLE_LOG_TIME_ROTATE_UNIT_DAY  'd' //!< rotate unit - day

typedef struct muggle_log_file_time_rot_handler
{
	muggle_log_handler_t handler;
	char                 filepath[MUGGLE_MAX_PATH];
	FILE                 *fp;
	char                 rotate_unit;
	unsigned int         rotate_mod;
	time_t               last_sec;
	struct tm            last_tm;
	bool                 use_local_time;
}muggle_log_file_time_rot_handler_t;

/**
 * @brief initialize file time rotate log handler
 *
 * @param handler        file time rotate log handler pointer
 * @param filepath       file path
 * @param rotate_unit    file rotate time unit, use macro MUGGLE_LOG_TIME_ROTATE_UNIT_*
 * @param rotate_mod     file rotate time mod
 * @param use_local_time if true, use local time, otherwise use UTC+0
 *
 * @return 
 *     - success returns 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_file_time_rot_handler_init(
	muggle_log_file_time_rot_handler_t *handler,
	const char *filepath,
	char rotate_unit,
	unsigned int rotate_mod,
	bool use_local_time);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_LOG_FILE_TIME_ROTATE_HANDLER_H_ */
