/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "log_handle_file.h"
#include "muggle/c/base/err.h"

int muggle_log_handle_file_init(
	muggle_log_handle_t *handle,
	int write_type,
	int fmt_flag,
	int level,
	muggle_atomic_int async_capacity,
	const char *file_path)
{
	handle->type = MUGGLE_LOG_TYPE_FILE;
	int ret = muggle_log_handle_base_init(handle, write_type, fmt_flag, level, async_capacity);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	handle->file.fp = fopen(file_path, "ab");
	if (handle->file.fp == NULL)
	{
		return MUGGLE_ERR_SYS_CALL;
	}	

	return MUGGLE_OK;
}

int muggle_log_handle_file_destroy(muggle_log_handle_t *handle)
{
	if (handle->file.fp)
	{
		fclose(handle->file.fp);
		handle->file.fp = NULL;
	}

	return MUGGLE_OK;
}

int muggle_log_handle_file_output(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
)
{
	int ret;
	char buf[MUGGLE_LOG_MAX_LEN] = { 0 };

	ret = muggle_log_fmt_gen(handle->fmt_flag, arg, msg, buf, sizeof(buf));
	if (ret <= 0)
	{
		return ret;
	}

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_lock(&handle->sync.mutex);
	}

	ret = (int)fwrite(buf, 1, ret, handle->file.fp);

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_unlock(&handle->sync.mutex);
	}

	return ret;
}
