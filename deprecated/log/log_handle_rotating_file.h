/******************************************************************************
 *  @file         log_handle_rotating_file.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec rotating file log handle
 *****************************************************************************/
 
#ifndef MUGGLE_C_LOG_HANDLE_ROTATING_FILE_H_
#define MUGGLE_C_LOG_HANDLE_ROTATING_FILE_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/log/log_fmt.h"
#include "muggle/c/log/log_handle.h"

EXTERN_C_BEGIN

/**
 * @brief initialize a rotating file log handle
 *
 * @param handle       rotating file log handle pointer
 * @param write_type   use one of MUGGLE_LOG_WRITE_TYPE_*
 * @param fmt_flag     use MUGGLE_LOG_FMT_*
 * @param level        log level that the log handle will output
 * @param async_capacity  if write_type == MUGGLE_LOG_WRITE_TYPE_ASYNC, use this specify async buffer capacity
 * @param p_alloc      function for async allocate memory, if NULL, use malloc
 * @param p_free       function for async free memory, if NULL, use free
 * @param file_path    log file path
 * @param max_bytes    max size when to rotate log
 * @param backup_count max backup file count
 *
 * @return success returns 0, otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_handle_rotating_file_init(
	muggle_log_handle_t *handle,
	int write_type,
	int fmt_flag,
	int level,
	muggle_atomic_int async_capacity,
	muggle_log_handle_async_alloc p_alloc,
	muggle_log_handle_async_free p_free,
	const char *file_path,
	unsigned int max_bytes,
	unsigned int backup_count);

/**
 * @brief  destroy a rotating file log handle
 *
 * NOTE: don't invoke this function immediatly, use muggle_log_handle_destroy
 *
 * @param handle rotating file log handle pointer
 *
 * @return success returns 0, otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_handle_rotating_file_destroy(muggle_log_handle_t *handle);

/**
 * @brief output message
 *
 * NOTE: don't invoke this function immediatly, use muggle_log_handle_write
 *
 * @param handle  rotating file log handle pointer
 * @param arg     log format arguments
 * @param msg     log messages
 *
 * @return success return number of bytes be writed to output, otherwise return negative
 */
MUGGLE_C_EXPORT
int muggle_log_handle_rotating_file_output(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
);

EXTERN_C_END

#endif
