#include "log_handle_file.h"
#include "muggle/c/base/err.h"

int muggle_log_handle_file_init(
	muggle_log_handle_t *handle,
	int write_type,
	int fmt_flag,
	muggle_atomic_int async_capacity,
	const char *file_path)
{
	handle->type = MUGGLE_LOG_TYPE_FILE;
	if (write_type >= MUGGLE_LOG_WRITE_TYPE_MAX || write_type < 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}
	handle->write_type = write_type;
	handle->fmt_flag = fmt_flag;
	async_capacity = async_capacity <= 8 ? 1024 * 8 : async_capacity;

	switch (write_type)
	{
		case MUGGLE_LOG_WRITE_TYPE_SYNC:
		{
			muggle_mutex_init(&handle->mutex);
		}break;
		case MUGGLE_LOG_WRITE_TYPE_ASYNC:
		{
			muggle_ringbuffer_init(
				&handle->ring,
				async_capacity,
				MUGGLE_RINGBUFFER_FLAG_WRITE_LOCK | MUGGLE_RINGBUFFER_FLAG_SINGLE_READER); 
			muggle_thread_create(&handle->thread, muggle_log_handle_run_async, handle);
		}break;
	}

	handle->fp = fopen(file_path, "ab");
	if (handle->fp == NULL)
	{
		return MUGGLE_ERR_SYS_CALL;
	}	

	return MUGGLE_OK;
}

int muggle_log_handle_file_destroy(muggle_log_handle_t *handle)
{
	switch (handle->write_type)
	{
		case MUGGLE_LOG_WRITE_TYPE_SYNC:
		{
			muggle_mutex_destroy(&handle->mutex);
		}break;
		case MUGGLE_LOG_WRITE_TYPE_ASYNC:
		{
			muggle_ringbuffer_write(&handle->ring, NULL);
			muggle_thread_join(&handle->thread);
		}break;
	}

	if (handle->fp)
	{
		fclose(handle->fp);
		handle->fp = NULL;
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
	if (ret < 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_lock(&handle->mutex);
	}

	fwrite(buf, 1, ret, handle->fp);

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_unlock(&handle->mutex);
	}

	return MUGGLE_OK;
}
