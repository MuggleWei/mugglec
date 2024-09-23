#include "log_file_handler.h"
#include <string.h>
#include "muggle/c/log/log_handler.h"
#include "muggle/c/base/err.h"
#include "muggle/c/os/os.h"
#include "muggle/c/os/path.h"

/**
 * @brief write log
 *
 * @param handler  log handler pointer
 * @param msg      muggle log msg
 *
 * @return
 *     - on success, return number of bytes be writed
 *     - otherwise return negative number
 */
static int muggle_log_file_handler_write(
	struct muggle_log_handler *base_handler, const muggle_log_msg_t *msg)
{
	char buf[MUGGLE_LOG_MSG_MAX_LEN];
	muggle_log_fmt_t *fmt = muggle_log_handler_get_fmt(base_handler);
	if (fmt == NULL)
	{
		return -1;
	}

	int ret = fmt->fmt_func(msg, buf, sizeof(buf));
	if (ret < 0)
	{
		return -2;
	}

	muggle_log_file_handler_t *handler = (muggle_log_file_handler_t*)base_handler;

	if (base_handler->need_mutex)
	{
		muggle_mutex_lock(&base_handler->mtx);
	}

	if (handler->fp)
	{
		ret = (int)fwrite(buf, 1, ret, handler->fp);
		fflush(handler->fp);
	}

	if (base_handler->need_mutex)
	{
		muggle_mutex_unlock(&base_handler->mtx);
	}

	return ret;
}

/**
 * @brief destroy file log handler
 *
 * @param handler  log handler pointer
 *
 * @return
 *     - success returns 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
static int muggle_log_file_handler_destroy(
	muggle_log_handler_t *base_handler)
{
	muggle_log_file_handler_t *handler = (muggle_log_file_handler_t*)base_handler;
	if (handler->fp != NULL)
	{
		fclose(handler->fp);
		handler->fp = NULL;
	}

	return muggle_log_handler_destroy_default(base_handler);
}

int muggle_log_file_handler_init(
	muggle_log_file_handler_t *handler,
	const char *filepath,
	const char *mode)
{
	memset(handler, 0, sizeof(*handler));

	int ret = muggle_log_handler_init_default((muggle_log_handler_t*)handler);
	if (ret != 0)
	{
		return ret;
	}

	handler->handler.write = muggle_log_file_handler_write;
	handler->handler.destroy = muggle_log_file_handler_destroy;

	const char* abs_filepath = NULL;
	char log_path[MUGGLE_MAX_PATH];
	if (muggle_path_isabs(filepath))
	{
		abs_filepath = filepath;
	}
	else
	{
		char cur_path[MUGGLE_MAX_PATH];
		ret = muggle_os_curdir(cur_path, sizeof(cur_path));
		if (ret != 0)
		{
			return ret;
		}

		ret = muggle_path_join(cur_path, filepath, log_path, sizeof(log_path));
		if (ret != 0)
		{
			return ret;
		}

		abs_filepath = log_path;
	}

	handler->fp = muggle_os_fopen(abs_filepath, mode);
	if (handler->fp == NULL)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	return MUGGLE_OK;
}
