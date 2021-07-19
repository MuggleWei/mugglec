/******************************************************************************
 *  @file         log_file_rotate_handler.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log file rotate handler
 *****************************************************************************/

#ifndef MUGGLE_C_FILE_ROTATE_HANDLER_H_
#define MUGGLE_C_FILE_ROTATE_HANDLER_H_

#include "muggle/c/base/macro.h"
#include <stdio.h>
#include "muggle/c/log/log_handler.h"

EXTERN_C_BEGIN

/**
 * @brief muggle file rotate handler
 */
typedef struct muggle_log_file_rotate_handler
{
	muggle_log_handler_t handler;
	char                 filepath[MUGGLE_MAX_PATH];
	FILE                 *fp;
	unsigned int         max_bytes;
	unsigned int         backup_count;
	long                 offset;
}muggle_log_file_rotate_handler_t;

/**
 * @brief initialize file rotate log handler
 *
 * @param handler       file rotate log handler pointer
 * @param filepath      file path
 * @param max_bytes     max bytes per file
 * @param backup_count  max backup file count
 *
 * @return 
 *     - success returns 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_file_rotate_handler_init(
	muggle_log_file_rotate_handler_t *handler,
	const char *filepath,
	unsigned int max_bytes,
	unsigned int backup_count);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_FILE_ROTATE_HANDLER_H_ */
