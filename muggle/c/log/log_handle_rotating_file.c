/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "log_handle_rotating_file.h"
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/os/path.h"
#include "muggle/c/os/os.h"

static int muggle_log_handle_rotating_file_detect(muggle_log_handle_t *handle)
{
	if (handle->rotating_file.offset >= handle->rotating_file.max_bytes)
	{
		return 1;
	}
	return 0;
}

static int muggle_log_handle_rotating_file_rotate(muggle_log_handle_t *handle)
{
	char buf[MUGGLE_MAX_PATH];

	if (handle->rotating_file.fp)
	{
		fclose(handle->rotating_file.fp);
	}

	snprintf(buf, sizeof(buf)-1, "%s.%d",
		handle->rotating_file.path, handle->rotating_file.backup_count);
	if (muggle_path_exists(buf))
	{
		muggle_os_remove(buf);
	}

	char src[MUGGLE_MAX_PATH], dst[MUGGLE_MAX_PATH];
	for (int i = (int)handle->rotating_file.backup_count - 1; i > 0; i--)
	{
		snprintf(src, sizeof(src), "%s.%d", handle->rotating_file.path, i);
		snprintf(dst, sizeof(dst), "%s.%d", handle->rotating_file.path, i+1);
		muggle_os_rename(src, dst);
	}

	snprintf(dst, sizeof(dst), "%s.1", handle->rotating_file.path);
	muggle_os_rename(handle->rotating_file.path, dst);

	handle->rotating_file.fp = fopen(handle->rotating_file.path, "ab+");
	if (handle->rotating_file.fp == NULL)
	{
		return MUGGLE_ERR_SYS_CALL;
	}	

	handle->rotating_file.offset = 0;

	return MUGGLE_OK;
}

int muggle_log_handle_rotating_file_init(
	muggle_log_handle_t *handle,
	int write_type,
	int fmt_flag,
	muggle_atomic_int async_capacity,
	const char *file_path,
	unsigned int max_bytes,
	unsigned int backup_count)
{
	handle->type = MUGGLE_LOG_TYPE_ROTATING_FILE;
	int ret = muggle_log_handle_base_init(handle, write_type, fmt_flag, async_capacity);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	handle->rotating_file.fp = fopen(file_path, "ab+");
	if (handle->rotating_file.fp == NULL)
	{
		return MUGGLE_ERR_SYS_CALL;
	}	

	strncpy(handle->rotating_file.path, file_path, sizeof(handle->rotating_file.path)-1);
	handle->rotating_file.max_bytes = max_bytes;
	handle->rotating_file.backup_count = backup_count;
	fseek(handle->rotating_file.fp, 0, SEEK_END);
	handle->rotating_file.offset = ftell(handle->rotating_file.fp);

	if (muggle_log_handle_rotating_file_detect(handle))
	{
		muggle_log_handle_rotating_file_rotate(handle);
	}

	return MUGGLE_OK;
}

int muggle_log_handle_rotating_file_destroy(muggle_log_handle_t *handle)
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

	if (handle->rotating_file.fp)
	{
		fclose(handle->rotating_file.fp);
		handle->rotating_file.fp = NULL;
	}

	return MUGGLE_OK;
}

int muggle_log_handle_rotating_file_output(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
)
{
	int ret;
	char buf[MUGGLE_LOG_MAX_LEN] = { 0 };

	ret = muggle_log_fmt_gen(handle->fmt_flag, arg, msg, buf, sizeof(buf));
	if (ret < 0)
	{
		return ret;
	}

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_lock(&handle->sync.mutex);
	}

	if (handle->rotating_file.fp)
	{
		fwrite(buf, 1, ret, handle->rotating_file.fp);
	}

	handle->rotating_file.offset += ret;
	if (muggle_log_handle_rotating_file_detect(handle))
	{
		muggle_log_handle_rotating_file_rotate(handle);
	}

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_unlock(&handle->sync.mutex);
	}

	return ret;
}
