/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_LOG_HANDLE_H_
#define MUGGLE_C_LOG_HANDLE_H_

#include <stdio.h>
#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/sync/mutex.h"
#include "muggle/c/sync/ringbuffer.h"
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
	MUGGLE_LOG_WRITE_TYPE_DEFAULT = 0, // write without protect
	MUGGLE_LOG_WRITE_TYPE_SYNC, // sync write with mutex
	MUGGLE_LOG_WRITE_TYPE_ASYNC, // async write
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
	char msg[MUGGLE_LOG_MAX_LEN];
}muggle_log_asnyc_msg_t;

typedef struct muggle_log_handle_property_sync_tag
{
	muggle_mutex_t mutex;
}muggle_log_handle_property_sync_t;

typedef struct muggle_log_handle_property_async_tag
{
	muggle_ringbuffer_t ring;
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

typedef struct muggle_log_handle_tag
{
	int type;
	int write_type;
	int fmt_flag;
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

/*
 * common initialize for log handle
 * NOTE: don't invoke this function immediatly
 * */
MUGGLE_CC_EXPORT
int muggle_log_handle_base_init(
	muggle_log_handle_t *handle,
	int write_type,
	int fmt_flag,
	muggle_atomic_int async_capacity
);

/*
 * destroy a log handle
 * @handle: console log handle pointer
 * RETURN: success returns 0, otherwise return err code in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_log_handle_destroy(muggle_log_handle_t *handle);

/*
 * output message
 * @handle: console log handle pointer
 * @arg: log format arguments
 * @msg: log messages
 * RETURN: success returns 0, otherwise return err code in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_log_handle_write(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
);

EXTERN_C_END

#endif
