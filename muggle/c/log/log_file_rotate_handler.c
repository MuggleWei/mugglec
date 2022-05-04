#include "log_file_rotate_handler.h"
#include <string.h>
#include "muggle/c/os/path.h"
#include "muggle/c/os/os.h"
#include "muggle/c/base/err.h"
#if MUGGLE_PLATFORM_WINDOWS
#include "muggle/c/time/win_gmtime.h"
#endif

/**
 * @brief rotate file rotate handler
 *
 * @param handler file rotate handler pointer
 *
 * @return 
 *     - success returns 0
 *     - otherwise return err code in muggle/c/base/err.h
 */
static int muggle_log_file_rotate_handler_rotate(muggle_log_file_rotate_handler_t *handler)
{
	if (handler->fp)
	{
		fclose(handler->fp);
		handler->fp = NULL;
	}

	char buf[MUGGLE_MAX_PATH];
	snprintf(buf, sizeof(buf)-1, "%s.%d",
		handler->filepath, handler->backup_count);
	if (muggle_path_exists(buf))
	{
		muggle_os_remove(buf);
	}

	char src[MUGGLE_MAX_PATH], dst[MUGGLE_MAX_PATH];
	for (int i = (int)handler->backup_count - 1; i > 0; i--)
	{
		snprintf(src, sizeof(src), "%s.%d", handler->filepath, i);
		snprintf(dst, sizeof(dst), "%s.%d", handler->filepath, i+1);
		muggle_os_rename(src, dst);
	}

	snprintf(dst, sizeof(dst), "%s.1", handler->filepath);
	muggle_os_rename(handler->filepath, dst);

	handler->fp = fopen(handler->filepath, "ab+");
	if (handler->fp == NULL)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	handler->offset = 0;

	return MUGGLE_OK;
}

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
static int muggle_log_file_rotate_handler_write(
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

	muggle_log_file_rotate_handler_t *handler = (muggle_log_file_rotate_handler_t*)base_handler;

	if (base_handler->need_mutex)
	{
		muggle_mutex_lock(&base_handler->mtx);
	}

	if (handler->fp)
	{
		ret = (int)fwrite(buf, 1, ret, handler->fp);
		fflush(handler->fp);

		handler->offset += (long)ret;
		if (handler->offset >= handler->max_bytes)
		{
			if (muggle_log_file_rotate_handler_rotate(handler) != 0)
			{
				fprintf(stderr, "failed rotate log handler");
			}
		}
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
static int muggle_log_file_rotate_handler_destroy(
	muggle_log_handler_t *base_handler)
{
	muggle_log_file_rotate_handler_t *handler = (muggle_log_file_rotate_handler_t*)base_handler;
	if (handler->fp != NULL)
	{
		fclose(handler->fp);
		handler->fp = NULL;
	}

	return muggle_log_handler_destroy_default(base_handler);
}

int muggle_log_file_rotate_handler_init(
	muggle_log_file_rotate_handler_t *handler,
	const char *filepath,
	unsigned int max_bytes,
	unsigned int backup_count)
{
	memset(handler, 0, sizeof(*handler));

	int ret = muggle_log_handler_init_default((muggle_log_handler_t*)handler);
	if (ret != 0)
	{
		return ret;
	}

	handler->handler.write = muggle_log_file_rotate_handler_write;
	handler->handler.destroy = muggle_log_file_rotate_handler_destroy;

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

		char log_dir[MUGGLE_MAX_PATH];
		ret = muggle_path_dirname(log_path, log_dir, sizeof(log_dir));
		if (ret != 0)
		{
			return ret;
		}

		if (!muggle_path_exists(log_dir))
		{
			ret = muggle_os_mkdir(log_dir);
			if (ret != 0)
			{
				return ret;
			}
		}

		abs_filepath = log_path;
	}

	handler->fp = fopen(abs_filepath, "ab+");
	if (handler->fp == NULL)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	strncpy(handler->filepath, abs_filepath, sizeof(handler->filepath)-1);
	handler->max_bytes = max_bytes;
	handler->backup_count = backup_count;

	fseek(handler->fp, 0, SEEK_END);
	handler->offset = ftell(handler->fp);

	if (handler->offset >= handler->max_bytes)
	{
		muggle_log_file_rotate_handler_rotate(handler);
	}

	return MUGGLE_OK;
}
