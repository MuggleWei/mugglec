/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "log_handle.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/log/log_handle_console.h"

typedef int (*muggle_log_output_fn)(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
);
typedef int (*muggle_log_handle_destroy_fn)(muggle_log_handle_t *handle);

static muggle_log_output_fn s_output_fn[MUGGLE_LOG_TYPE_MAX] = {
	muggle_log_handle_console_output,
	// TODO:
};

static muggle_log_handle_destroy_fn s_log_handle_destroy_fn[MUGGLE_LOG_TYPE_MAX] = {
	muggle_log_handle_console_destroy,
	// TODO:
};

static int muggle_log_handle_async_write(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
)
{
	muggle_log_asnyc_msg_t *async_msg = (muggle_log_asnyc_msg_t*)malloc(sizeof(muggle_log_asnyc_msg_t));
	if (async_msg == NULL)
	{
		return MUGGLE_ERR_MEM_ALLOC;
	}

	async_msg->level = arg->level;
	async_msg->line = arg->line;
	strncpy(async_msg->file, arg->file, sizeof(async_msg->file) - 1);
	strncpy(async_msg->func, arg->file, sizeof(async_msg->func) - 1);
	strncpy(async_msg->msg, msg, sizeof(async_msg->msg) - 1);

	return muggle_ringbuffer_write(&handle->ring, async_msg);
}

muggle_thread_ret_t muggle_log_handle_run_async(void *arg)
{
	int idx = 0;
	muggle_log_handle_t *handle = (muggle_log_handle_t*)arg;
	while (1)
	{
		muggle_log_asnyc_msg_t *async_msg = 
			(muggle_log_asnyc_msg_t*)muggle_ringbuffer_read(&handle->ring, idx++);
		if (async_msg == NULL)
		{
			break;
		}

		muggle_log_fmt_arg_t arg = {
			async_msg->level,
			async_msg->line,
			async_msg->file,
			async_msg->func
		};
		s_output_fn[handle->type](handle, &arg, async_msg->msg);
	}

	return 0;
}

int muggle_log_handle_destroy(muggle_log_handle_t *handle)
{
	return s_log_handle_destroy_fn[handle->type](handle);
}

int muggle_log_handle_write(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
)
{
	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_ASYNC)
	{
		return muggle_log_handle_async_write(handle, arg, msg);
	}
	else
	{
		return s_output_fn[handle->type](handle, arg, msg);
	}
}
