#include "log_file_time_rot_handler.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/os/path.h"
#include "muggle/c/os/os.h"
#include "muggle/c/base/err.h"

/**
 * @brief
 * detect whether or not need rotate file, and update 
 * handler info
 *
 *
 * @param handler  file time roate handler
 * @param msg      log message
 *
 * @return 
 */
static bool muggle_log_file_time_rot_handler_detect(
	muggle_log_file_time_rot_handler_t *handler,
	const muggle_log_msg_t *msg)
{
	time_t sec = msg->ts.tv_sec;
	if (sec == 0)
	{
		sec = time(NULL);
	}

	if (handler->last_sec >= sec)
	{
		return false;
	}

	struct tm curr_tm;
	if (handler->use_local_time)
	{
		localtime_r(&sec, &curr_tm);
	}
	else
	{
		gmtime_r(&sec, &curr_tm);
	}

	bool need_rot = false;
	switch (handler->rotate_unit)
	{
		case MUGGLE_LOG_TIME_ROTATE_UNIT_SEC:
		{
			int curr_sec_mod = curr_tm.tm_sec / handler->rotate_mod;
			int last_sec_mod = handler->last_tm.tm_sec / handler->rotate_mod;
			if (curr_sec_mod != last_sec_mod ||
				curr_tm.tm_min != handler->last_tm.tm_min ||
				curr_tm.tm_hour != handler->last_tm.tm_hour ||
				curr_tm.tm_mday != handler->last_tm.tm_mday ||
				curr_tm.tm_mon != handler->last_tm.tm_mon ||
				curr_tm.tm_year != handler->last_tm.tm_year)
			{
				need_rot = true;
			}
		}break;
		case MUGGLE_LOG_TIME_ROTATE_UNIT_MIN:
		{
			int curr_min_mod = curr_tm.tm_min / handler->rotate_mod;
			int last_min_mod = handler->last_tm.tm_min / handler->rotate_mod;
			if (curr_min_mod != last_min_mod ||
				curr_tm.tm_hour != handler->last_tm.tm_hour ||
				curr_tm.tm_mday != handler->last_tm.tm_mday ||
				curr_tm.tm_mon != handler->last_tm.tm_mon ||
				curr_tm.tm_year != handler->last_tm.tm_year)
			{
				need_rot = true;
			}
		}break;
		case MUGGLE_LOG_TIME_ROTATE_UNIT_HOUR:
		{
			int curr_hour_mod = curr_tm.tm_hour / handler->rotate_mod;
			int last_hour_mod = handler->last_tm.tm_hour / handler->rotate_mod;
			if (curr_hour_mod != last_hour_mod ||
				curr_tm.tm_mday != handler->last_tm.tm_mday ||
				curr_tm.tm_mon != handler->last_tm.tm_mon ||
				curr_tm.tm_year != handler->last_tm.tm_year)
			{
				need_rot = true;
			}
		}break;
		case MUGGLE_LOG_TIME_ROTATE_UNIT_DAY:
		{
			int curr_day_mod = curr_tm.tm_mday / handler->rotate_mod;
			int last_day_mod = handler->last_tm.tm_mday / handler->rotate_mod;
			if (curr_day_mod != last_day_mod ||
				curr_tm.tm_mon != handler->last_tm.tm_mon ||
				curr_tm.tm_year != handler->last_tm.tm_year)
			{
				need_rot = true;
			}
		}break;
	}

	handler->last_sec = sec;
	memcpy(&handler->last_tm, &curr_tm, sizeof(handler->last_tm));

	return need_rot;
}

static int muggle_log_file_time_rot_handler_rotate(
	muggle_log_file_time_rot_handler_t *handler)
{
	if (handler->fp)
	{
		fclose(handler->fp);
		handler->fp = NULL;
	}

	int ret = 0;
	char buf[MUGGLE_MAX_PATH];
	switch (handler->rotate_unit)
	{
		case MUGGLE_LOG_TIME_ROTATE_UNIT_SEC:
		{
			ret = snprintf(buf, sizeof(buf), "%s.%d%02d%02dT%02d%02d%02d",
				handler->filepath,
				handler->last_tm.tm_year + 1900,
				handler->last_tm.tm_mon + 1,
				handler->last_tm.tm_mday,
				handler->last_tm.tm_hour,
				handler->last_tm.tm_min,
				handler->last_tm.tm_sec);

		}break;
		case MUGGLE_LOG_TIME_ROTATE_UNIT_MIN:
		{
			ret = snprintf(buf, sizeof(buf), "%s.%d%02d%02dT%02d%02d",
				handler->filepath,
				handler->last_tm.tm_year + 1900,
				handler->last_tm.tm_mon + 1,
				handler->last_tm.tm_mday,
				handler->last_tm.tm_hour,
				handler->last_tm.tm_min);
		}break;
		case MUGGLE_LOG_TIME_ROTATE_UNIT_HOUR:
		{
			ret = snprintf(buf, sizeof(buf), "%s.%d%02d%02dT%02d",
				handler->filepath,
				handler->last_tm.tm_year + 1900,
				handler->last_tm.tm_mon + 1,
				handler->last_tm.tm_mday,
				handler->last_tm.tm_hour);
		}break;
		case MUGGLE_LOG_TIME_ROTATE_UNIT_DAY:
		{
			ret = snprintf(buf, sizeof(buf), "%s.%d%02d%02d",
				handler->filepath,
				handler->last_tm.tm_year + 1900,
				handler->last_tm.tm_mon + 1,
				handler->last_tm.tm_mday);
		}break;
	}
	if (ret < 0) {
		fprintf(stderr, "failed snprintf time rotate log buf\n");
		return MUGGLE_ERR_SYS_CALL;
	}

	handler->fp = muggle_os_fopen(buf, "ab+");
	if (handler->fp == NULL)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

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
static int muggle_log_file_time_rot_handler_write(
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

	muggle_log_file_time_rot_handler_t *handler = (muggle_log_file_time_rot_handler_t*)base_handler;

	if (base_handler->need_mutex)
	{
		muggle_mutex_lock(&base_handler->mtx);
	}

	if (handler->fp)
	{
		ret = (int)fwrite(buf, 1, ret, handler->fp);
		fflush(handler->fp);

		if (muggle_log_file_time_rot_handler_detect(handler, msg))
		{
			if (muggle_log_file_time_rot_handler_rotate(handler) != 0)
			{
				fprintf(stderr, "failed rotate log handler\n");
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
static int muggle_log_file_time_rot_handler_destroy(
	muggle_log_handler_t *base_handler)
{
	muggle_log_file_time_rot_handler_t *handler = (muggle_log_file_time_rot_handler_t*)base_handler;
	if (handler->fp != NULL)
	{
		fclose(handler->fp);
		handler->fp = NULL;
	}

	return muggle_log_handler_destroy_default(base_handler);
}

int muggle_log_file_time_rot_handler_init(
	muggle_log_file_time_rot_handler_t *handler,
	const char *filepath,
	char rotate_unit,
	unsigned int rotate_mod,
	bool use_local_time)
{
	memset(handler, 0, sizeof(*handler));

	int ret = muggle_log_handler_init_default((muggle_log_handler_t*)handler);
	if (ret != 0)
	{
		return ret;
	}

	handler->handler.write = muggle_log_file_time_rot_handler_write;
	handler->handler.destroy = muggle_log_file_time_rot_handler_destroy;

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

	strncpy(handler->filepath, abs_filepath, sizeof(handler->filepath)-1);
	// handler already memset, the line below just for get rid of gcc strncpy 
	// truncated warning
	handler->filepath[sizeof(handler->filepath) - 1] = '\0';

	handler->last_sec = time(NULL);
	if (handler->use_local_time)
	{
		localtime_r(&handler->last_sec, &handler->last_tm);
	}
	else
	{
		gmtime_r(&handler->last_sec, &handler->last_tm);
	}
	handler->rotate_mod = rotate_mod;
	handler->rotate_unit = rotate_unit;
	handler->use_local_time = use_local_time;

	ret = muggle_log_file_time_rot_handler_rotate(handler);
	if (ret != 0)
	{
		return ret;
	}

	return MUGGLE_OK;
}
