#include "muggle/c/muggle_c.h"

muggle_logger_t* my_async_logger()
{
	static muggle_async_logger_t async_logger;
	return (muggle_logger_t*)&async_logger;
}

#define MY_LOG(level, format, ...) \
do \
{ \
	muggle_log_src_loc_t loc_arg##__LINE__ = { \
		__FILE__, __LINE__, __FUNCTION__ \
	}; \
	muggle_logger_t *logger = my_async_logger(); \
	logger->log(logger, level, &loc_arg##__LINE__, format, ##__VA_ARGS__); \
} while (0)

#define MY_LOG_TRACE(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_TRACE, format, ##__VA_ARGS__)
#define MY_LOG_DEBUG(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define MY_LOG_INFO(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define MY_LOG_WARNING(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define MY_LOG_ERROR(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define MY_LOG_FATAL(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_FATAL, format, ##__VA_ARGS__)

int my_log_fmt_func(const muggle_log_msg_t *msg, char *buf, size_t bufsize)
{
	const char *level = muggle_log_level_to_str(msg->level);

	char filename[MUGGLE_MAX_PATH];
	muggle_path_basename(msg->src_loc.file, filename, sizeof(filename));

	struct tm t;
	localtime_r(&msg->ts.tv_sec, &t);

	const char *payload = "";
	if (msg->payload)
	{
		payload = msg->payload;
	}

	return (int)snprintf(buf, bufsize,
		"%s|%d-%02d-%02dT%02d:%02d:%02d.%03d|%s.%u|%llu - %s\n",
		level,
		(int)t.tm_year+1900, (int)t.tm_mon+1, (int)t.tm_mday,
		(int)t.tm_hour, (int)t.tm_min, (int)t.tm_sec,
		(int)msg->ts.tv_nsec / 1000000,
		filename, (unsigned int)msg->src_loc.line,
		(unsigned long long)msg->tid,
		payload);
}

void init_log()
{
	muggle_logger_t *logger = my_async_logger();
	muggle_async_logger_init((muggle_async_logger_t*)logger, 4096);

	// fmt
	static muggle_log_fmt_t formatter = {
		MUGGLE_LOG_FMT_ALL,
		my_log_fmt_func
	};

	// console handler
	static muggle_log_console_handler_t console_handler;
	muggle_log_console_handler_init(&console_handler, 1);
	muggle_log_handler_set_fmt((muggle_log_handler_t*)&console_handler, &formatter);
	muggle_log_handler_set_level((muggle_log_handler_t*)&console_handler,
		MUGGLE_LOG_LEVEL_WARNING);

	// file time rotate handler
	static muggle_log_file_time_rot_handler_t time_rot_handler;
	muggle_log_file_time_rot_handler_init(
		&time_rot_handler, "log/log_customize.log",
		MUGGLE_LOG_TIME_ROTATE_UNIT_DAY, 1, true);
	muggle_log_handler_set_fmt((muggle_log_handler_t*)&time_rot_handler, &formatter);
	muggle_log_handler_set_level((muggle_log_handler_t*)&time_rot_handler,
		MUGGLE_LOG_LEVEL_TRACE);

	// add handler
	logger->add_handler(logger, (muggle_log_handler_t*)&console_handler);
	logger->add_handler(logger, (muggle_log_handler_t*)&time_rot_handler);
}

void destroy_log()
{
	muggle_logger_t *logger = my_async_logger();
	logger->destroy(logger);
}

int main()
{
	init_log();

	MY_LOG_TRACE("my logger - trace");
	MY_LOG_DEBUG("my logger - debug");
	MY_LOG_INFO("my logger - info");
	MY_LOG_WARNING("my logger - warning");
	MY_LOG_ERROR("my logger - error");

	destroy_log();

	return 0;
}