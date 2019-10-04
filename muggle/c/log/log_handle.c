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
#include "muggle/c/log/log_handle_file.h"
#include "muggle/c/log/log_handle_rotating_file.h"
#include "muggle/c/log/log_handle_win_debug.h"

typedef int (*muggle_log_output_fn)(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
);
typedef int (*muggle_log_handle_destroy_fn)(muggle_log_handle_t *handle);

static muggle_log_output_fn s_output_fn[MUGGLE_LOG_TYPE_MAX] = {
	muggle_log_handle_console_output,
	muggle_log_handle_file_output,
	muggle_log_handle_rotating_file_output,
	muggle_log_handle_win_debug_output,
};

static muggle_log_handle_destroy_fn s_log_handle_destroy_fn[MUGGLE_LOG_TYPE_MAX] = {
	muggle_log_handle_console_destroy,
	muggle_log_handle_file_destroy,
	muggle_log_handle_rotating_file_destroy,
	muggle_log_handle_win_debug_destroy,
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

	return muggle_ringbuffer_write(&handle->async.ring, async_msg);
}

muggle_thread_ret_t muggle_log_handle_run_async(void *arg)
{
	int idx = 0;
	muggle_log_handle_t *handle = (muggle_log_handle_t*)arg;
	while (1)
	{
		muggle_log_asnyc_msg_t *async_msg = 
			(muggle_log_asnyc_msg_t*)muggle_ringbuffer_read(&handle->async.ring, idx++);
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

		free(async_msg);
	}

	return 0;
}

int muggle_log_handle_base_init(
	muggle_log_handle_t *handle,
	int write_type,
	int fmt_flag,
	int level,
	muggle_atomic_int async_capacity
)
{
	if (write_type >= MUGGLE_LOG_WRITE_TYPE_MAX || write_type < 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	handle->write_type = write_type;
	handle->fmt_flag = fmt_flag;
	handle->level = level;
	async_capacity = async_capacity <= 8 ? 1024 * 8 : async_capacity;

	switch (write_type)
	{
		case MUGGLE_LOG_WRITE_TYPE_SYNC:
		{
			muggle_mutex_init(&handle->sync.mutex);
		}break;
		case MUGGLE_LOG_WRITE_TYPE_ASYNC:
		{
			muggle_ringbuffer_init(
				&handle->async.ring,
				async_capacity,
				MUGGLE_RINGBUFFER_FLAG_WRITE_LOCK | MUGGLE_RINGBUFFER_FLAG_SINGLE_READER); 
			muggle_thread_create(&handle->async.thread, muggle_log_handle_run_async, handle);
		}break;
	}

	return MUGGLE_OK;
}

int muggle_log_handle_destroy(muggle_log_handle_t *handle)
{
	switch (handle->write_type)
	{
		case MUGGLE_LOG_WRITE_TYPE_SYNC:
		{
			muggle_mutex_destroy(&handle->sync.mutex);
		}break;
		case MUGGLE_LOG_WRITE_TYPE_ASYNC:
		{
			muggle_ringbuffer_write(&handle->async.ring, NULL);
			muggle_thread_join(&handle->async.thread);
		}break;
	}

	return s_log_handle_destroy_fn[handle->type](handle);
}

int muggle_log_handle_write(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
)
{
	if (arg->level < handle->level)
	{
		return MUGGLE_OK;
	}

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_ASYNC)
	{
		return muggle_log_handle_async_write(handle, arg, msg);
	}
	else
	{
		return s_output_fn[handle->type](handle, arg, msg) > 0 ? MUGGLE_OK : MUGGLE_ERR_INVALID_PARAM;
	}
}
