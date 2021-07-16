/******************************************************************************
 *  @file         log_handle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log handle
 *****************************************************************************/
 
#ifndef MUGGLE_C_LOG_HANDLE_H_
#define MUGGLE_C_LOG_HANDLE_H_

#include <stdio.h>
#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/sync/mutex.h"
#include "muggle/c/sync/ring_buffer.h"
#include "muggle/c/log/log_fmt.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_LOG_TYPE_CONSOLE = 0,
	MUGGLE_LOG_TYPE_FILE,
	MUGGLE_LOG_TYPE_ROTATING_FILE,
	MUGGLE_LOG_TYPE_WIN_DEBUG_OUT,
	MUGGLE_LOG_TYPE_MAX,
};

enum
{
	MUGGLE_LOG_WRITE_TYPE_DEFAULT = 0, //!< log write without protect
	MUGGLE_LOG_WRITE_TYPE_SYNC,        //!< log sync write with mutex
	MUGGLE_LOG_WRITE_TYPE_ASYNC,       //!< log async write
	MUGGLE_LOG_WRITE_TYPE_MAX,
};

enum
{
	MUGGLE_LOG_HANDLE_RESERVE_SIZE = 64,
	MUGGLE_LOG_MAX_LEN = 4096,
};

typedef struct muggle_log_async_msg_tag
{
	int level;
	unsigned int line;
	char file[512];
	char func[512];
	muggle_thread_id tid;
	char msg[MUGGLE_LOG_MAX_LEN];
}muggle_log_asnyc_msg_t;

typedef struct muggle_log_handle_property_sync_tag
{
	muggle_mutex_t mutex;
}muggle_log_handle_property_sync_t;

typedef struct muggle_log_handle_property_async_tag
{
	muggle_ring_buffer_t ring;
	muggle_thread_t thread;
}muggle_log_handle_property_async_t;

typedef struct muggle_log_handle_property_console_tag
{
	int enable_color;
}muggle_log_handle_property_console_t;

typedef struct muggle_log_handle_property_file_tag
{
	FILE *fp;
}muggle_log_handle_property_file_t;

typedef struct muggle_log_handle_property_rotating_file_tag
{
	FILE *fp;
	char path[MUGGLE_MAX_PATH];
	unsigned int max_bytes;
	unsigned int backup_count;
	long offset;
}muggle_log_handle_property_rotating_file_t;

typedef void* (*muggle_log_handle_async_alloc)(size_t size);
typedef void (*muggle_log_handle_async_free)(void *ptr);

typedef struct muggle_log_handle_tag
{
	int type;
	int write_type;
	int fmt_flag;
	int level;
	muggle_log_handle_async_alloc p_alloc;
	muggle_log_handle_async_free p_free;
	union
	{
		muggle_log_handle_property_sync_t sync;
		muggle_log_handle_property_async_t async;
	};
	union
	{
		muggle_log_handle_property_console_t console;
		muggle_log_handle_property_file_t file;
		muggle_log_handle_property_rotating_file_t rotating_file;
	};
}muggle_log_handle_t;

/**
 * @brief common initialize for log handle
 *
 * NOTE: don't invoke this function immediatly
 *
 * @param handle         log handle
 * @param write_type     use one of MUGGLE_LOG_WRITE_TYPE_*
 * @param fmt_flag       use MUGGLE_LOG_FMT_*
 * @param level          log level that the log handle will output
 * @param async_capacity if write_type == MUGGLE_LOG_WRITE_TYPE_ASYNC, use this specify async buffer capacity
 * @param p_alloc        function for async allocate memory, if NULL, use malloc
 * @param p_free         function for async free memory, if NULL, use free
 *
 * @return
 *     0 - success
 *     otherwise - return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_handle_base_init(
	muggle_log_handle_t *handle,
	int write_type,
	int fmt_flag,
	int level,
	muggle_atomic_int async_capacity,
	muggle_log_handle_async_alloc p_alloc,
	muggle_log_handle_async_free p_free
);

/**
 * @brief  destroy a log handle
 *
 * @param handle  console log handle pointer
 *
 * @return  success returns 0, otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_handle_destroy(muggle_log_handle_t *handle);

/**
 * @brief  output message
 *
 * @param handle log handle pointer
 * @param arg    log format arguments
 * @param msg    log messages
 *
 * @return  success returns 0, otherwise return err code in err.h
 */
MUGGLE_C_EXPORT
int muggle_log_handle_write(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
);

EXTERN_C_END

#endif
