/******************************************************************************
 *  @file         log_file_handler.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec file log handler
 *****************************************************************************/

#ifndef MUGGLE_C_LOG_FILE_HANDLER_H_
#define MUGGLE_C_LOG_FILE_HANDLER_H_

#include "muggle/c/base/macro.h"
#include <stdio.h>
#include "muggle/c/log/log_handler.h"

EXTERN_C_BEGIN

/**
 * @brief muggle file log handler
 */
typedef struct muggle_log_file_handler
{
	muggle_log_handler_t handler;
	FILE                 *fp;
}muggle_log_file_handler_t;

/**
 * @brief initialze a file log handler
 *
 * @param handler   file log handler pointer
 * @param filepath  file path
 * @param mode      file open mode
 *
 * @return 
 */
MUGGLE_C_EXPORT
int muggle_log_file_handler_init(
	muggle_log_file_handler_t *handler,
	const char *filepath,
	const char *mode);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_LOG_FILE_HANDLER_H_ */
