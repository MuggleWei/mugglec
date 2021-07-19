#include "muggle/c/base/sleep.h"
#include "muggle/c/log/log_sync_logger.h"
#include "muggle/c/muggle_c.h"
#include "muggle/c/version/version.h"

#define GEN_TMP_LOG_MSG(msg, s) \
muggle_log_msg_t msg = { \
	MUGGLE_LOG_LEVEL_INFO, \
	{0, 0}, \
	0, \
	{__FILE__, __LINE__, __FUNCTION__}, \
	s \
}; \
timespec_get(&msg.ts, TIME_UTC); \
msg.tid = muggle_thread_current_id(); \

void example_simple_log_fmt()
{
	muggle_log_fmt_t *fmt = muggle_log_fmt_get_simple();
	GEN_TMP_LOG_MSG(msg, "simple log fmt");

	char buf[MUGGLE_LOG_MSG_MAX_LEN];
	fmt->fmt_func(&msg, buf, sizeof(buf));
	fprintf(stdout, "%s", buf);
}

void example_complicated_log_fmt()
{
	muggle_log_fmt_t *fmt = muggle_log_fmt_get_complicated();
	GEN_TMP_LOG_MSG(msg, "complicated log fmt");

	char buf[MUGGLE_LOG_MSG_MAX_LEN];
	fmt->fmt_func(&msg, buf, sizeof(buf));
	fprintf(stdout, "complicated fmt result: %s", buf);
}

int customize_log_fmt_func(const muggle_log_msg_t *msg, char *buf, size_t bufsize)
{
	const char *level = muggle_log_level_to_str(msg->level);

	char filename[MUGGLE_MAX_PATH];
	muggle_path_basename(msg->src_loc.file, filename, sizeof(filename));

	const char *payload = "";
	if (msg->payload)
	{
		payload = msg->payload;
	}

	return (int)snprintf(buf, bufsize,
		"%s|%s:%u|%llu.%06d|%llu - %s\n",
		level,
		filename, (unsigned int)msg->src_loc.line,
		(unsigned long long)msg->ts.tv_sec,
		(int)msg->ts.tv_nsec / 1000,
		(unsigned long long)msg->tid,
		payload);
}
void example_customize_log_fmt()
{
	muggle_log_fmt_t formatter = {
		MUGGLE_LOG_FMT_ALL,
		customize_log_fmt_func
	};

	muggle_log_fmt_t *fmt = &formatter;
	GEN_TMP_LOG_MSG(msg, "customize log fmt");

	char buf[MUGGLE_LOG_MSG_MAX_LEN];
	fmt->fmt_func(&msg, buf, sizeof(buf));
	fprintf(stdout, "customize fmt result: %s", buf);
}

void example_log_console_handler()
{
	muggle_log_handler_t *handler = NULL;

	{
		GEN_TMP_LOG_MSG(msg, "example log console handler without color");

		muggle_log_console_handler_t console_handler;
		muggle_log_console_handler_init(&console_handler, 0);
		handler = (muggle_log_handler_t*)&console_handler;

		msg.level = MUGGLE_LOG_LEVEL_WARNING;
		handler->write(handler, &msg);

		handler->destroy(handler);
	}

	{
		GEN_TMP_LOG_MSG(msg, "example log console handler with color when level >= WARNING");

		muggle_log_console_handler_t console_handler;
		muggle_log_console_handler_init(&console_handler, 1);
		handler = (muggle_log_handler_t*)&console_handler;

		msg.level = MUGGLE_LOG_LEVEL_WARNING;
		handler->write(handler, &msg);

		handler->destroy(handler);
	}

	{
		GEN_TMP_LOG_MSG(msg, "example log console handler set customize fmt");

		muggle_log_console_handler_t console_handler;
		muggle_log_console_handler_init(&console_handler, 1);
		handler = (muggle_log_handler_t*)&console_handler;

		muggle_log_fmt_t formatter = {
			MUGGLE_LOG_FMT_ALL,
			customize_log_fmt_func
		};
		muggle_log_handler_set_fmt(handler, &formatter);

		msg.level = MUGGLE_LOG_LEVEL_INFO;
		handler->write(handler, &msg);
		msg.level = MUGGLE_LOG_LEVEL_WARNING;
		handler->write(handler, &msg);
		msg.level = MUGGLE_LOG_LEVEL_ERROR;
		handler->write(handler, &msg);

		handler->destroy(handler);
	}
}

void example_log_file_handler()
{
	muggle_log_handler_t *handler = NULL;

	{
		GEN_TMP_LOG_MSG(msg, "example log file handler, write into log/example_log.log");

		muggle_log_file_handler_t file_handler;
		muggle_log_file_handler_init(&file_handler, "log/example_log.log", "w");
		handler = (muggle_log_handler_t*)&file_handler;
		muggle_log_handler_set_level((muggle_log_handler_t*)&file_handler, MUGGLE_LOG_LEVEL_DEBUG);

		msg.level = MUGGLE_LOG_LEVEL_INFO;
		handler->write(handler, &msg);
		msg.level = MUGGLE_LOG_LEVEL_WARNING;
		handler->write(handler, &msg);
		msg.level = MUGGLE_LOG_LEVEL_ERROR;
		handler->write(handler, &msg);

		handler->destroy(handler);
	}

	{
		GEN_TMP_LOG_MSG(msg, "example log file handler, write into ./example_log.log");

		muggle_log_file_handler_t file_handler;
		muggle_log_file_handler_init(&file_handler, "./example_log.log", "w");
		handler = (muggle_log_handler_t*)&file_handler;
		muggle_log_handler_set_level((muggle_log_handler_t*)&file_handler, MUGGLE_LOG_LEVEL_DEBUG);

		msg.level = MUGGLE_LOG_LEVEL_INFO;
		handler->write(handler, &msg);
		msg.level = MUGGLE_LOG_LEVEL_WARNING;
		handler->write(handler, &msg);
		msg.level = MUGGLE_LOG_LEVEL_ERROR;
		handler->write(handler, &msg);

		handler->destroy(handler);
	}

	{
		GEN_TMP_LOG_MSG(msg, "example log file handler, append into log/example_log.log");

		muggle_log_file_handler_t file_handler;
		muggle_log_file_handler_init(&file_handler, "log/example_log.log", "a");
		handler = (muggle_log_handler_t*)&file_handler;
		muggle_log_handler_set_level((muggle_log_handler_t*)&file_handler, MUGGLE_LOG_LEVEL_DEBUG);

		msg.level = MUGGLE_LOG_LEVEL_INFO;
		handler->write(handler, &msg);
		msg.level = MUGGLE_LOG_LEVEL_WARNING;
		handler->write(handler, &msg);
		msg.level = MUGGLE_LOG_LEVEL_ERROR;
		handler->write(handler, &msg);

		handler->destroy(handler);
	}
}

void example_log_file_rotate_handler()
{
	muggle_log_handler_t *handler = NULL;

	{
		GEN_TMP_LOG_MSG(msg, "example log file rotate handler");

		muggle_log_file_rotate_handler_t file_rot_handler;
		muggle_log_file_rotate_handler_init(&file_rot_handler, "log/example_rot.log", 128, 5);
		handler = (muggle_log_handler_t*)&file_rot_handler;
		muggle_log_handler_set_level((muggle_log_handler_t*)&file_rot_handler, MUGGLE_LOG_LEVEL_DEBUG);

		msg.level = MUGGLE_LOG_LEVEL_INFO;
		for (int i = 0; i < 16; i++)
		{
			handler->write(handler, &msg);
		}

		handler->destroy(handler);
	}
}

void example_log_file_time_rot_handler()
{
	muggle_log_handler_t *handler = NULL;

	{
		GEN_TMP_LOG_MSG(msg, "example log file time rotate handler");

		muggle_log_file_time_rot_handler_t file_time_rot_handler;
		muggle_log_file_time_rot_handler_init(&file_time_rot_handler, "log/example_time_rot.log", MUGGLE_LOG_TIME_ROTATE_UNIT_HOUR, 2, false);
		handler = (muggle_log_handler_t*)&file_time_rot_handler;
		muggle_log_handler_set_level((muggle_log_handler_t*)&file_time_rot_handler, MUGGLE_LOG_LEVEL_DEBUG);

		msg.level = MUGGLE_LOG_LEVEL_INFO;
		timespec_get(&msg.ts, TIME_UTC);
		for (int i = 0; i < 10; i++)
		{
			handler->write(handler, &msg);
			msg.ts.tv_sec += 60 * 60;
		}

		handler->destroy(handler);
	}
}

void example_logger_add_handler(muggle_logger_t *logger)
{
	// fmt
	static muggle_log_fmt_t formatter = {
		MUGGLE_LOG_FMT_ALL,
		customize_log_fmt_func
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
		&time_rot_handler, "log/example_logger.log",
		MUGGLE_LOG_TIME_ROTATE_UNIT_DAY, 1, false);
	muggle_log_handler_set_fmt((muggle_log_handler_t*)&time_rot_handler, &formatter);
	muggle_log_handler_set_level((muggle_log_handler_t*)&time_rot_handler,
		MUGGLE_LOG_LEVEL_TRACE);

	// add handler
	logger->add_handler(logger, (muggle_log_handler_t*)&console_handler);
	logger->add_handler(logger, (muggle_log_handler_t*)&time_rot_handler);
}

void example_sync_logger()
{
	// logger
	muggle_logger_t *logger = NULL;
	muggle_sync_logger_t sync_logger;
	muggle_sync_logger_init(&sync_logger);
	logger = (muggle_logger_t*)&sync_logger;

	example_logger_add_handler(logger);

	// log
	MUGGLE_LOG(logger, MUGGLE_LOG_LEVEL_TRACE, "example sync logger - trace");
	MUGGLE_LOG(logger, MUGGLE_LOG_LEVEL_DEBUG, "example sync logger - debug");
	MUGGLE_LOG(logger, MUGGLE_LOG_LEVEL_INFO, "example sync logger - info");
	MUGGLE_LOG(logger, MUGGLE_LOG_LEVEL_WARNING, "example sync logger - warning");
	MUGGLE_LOG(logger, MUGGLE_LOG_LEVEL_ERROR, "example sync logger - error");

	logger->destroy(logger);
}

void example_async_logger()
{
	// logger
	muggle_logger_t *logger = NULL;
	muggle_async_logger_t async_logger;
	muggle_async_logger_init(&async_logger, 4096);
	logger = (muggle_logger_t*)&async_logger;

	example_logger_add_handler(logger);

	// log
	MUGGLE_LOG(logger, MUGGLE_LOG_LEVEL_TRACE, "example async logger - trace");
	MUGGLE_LOG(logger, MUGGLE_LOG_LEVEL_DEBUG, "example async logger - debug");
	MUGGLE_LOG(logger, MUGGLE_LOG_LEVEL_INFO, "example async logger - info");
	MUGGLE_LOG(logger, MUGGLE_LOG_LEVEL_WARNING, "example async logger - warning");
	MUGGLE_LOG(logger, MUGGLE_LOG_LEVEL_ERROR, "example async logger - error");

	logger->destroy(logger);
}

void example_simple_log()
{
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_TRACE);

	MUGGLE_LOG_INFO("mugglec version: %s", mugglec_version());

	MUGGLE_LOG_INFO("use log simple init, just set console log level and file log level");

	MUGGLE_DEBUG_LOG_TRACE("debug trace, see this line only when debug");
	MUGGLE_DEBUG_LOG_DEBUG("debug debug, see this line only when debug");
	MUGGLE_DEBUG_LOG_INFO("debug info, see this line only when debug");
	MUGGLE_DEBUG_LOG_WARNING("debug warning, see this line only when debug");
	MUGGLE_DEBUG_LOG_ERROR("debug error, see this line only when debug");

	const char *word = "log";
	MUGGLE_LOG_TRACE("%s trace", word);
	MUGGLE_LOG_DEBUG("%s debug", word);
	MUGGLE_LOG_INFO("%s info", word);
	MUGGLE_LOG_WARNING("%s warning", word);
	MUGGLE_LOG_ERROR("%s error", word);
}

muggle_logger_t* customize_async_logger()
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
	muggle_logger_t *logger = customize_async_logger(); \
	logger->log(logger, level, &loc_arg##__LINE__, format, ##__VA_ARGS__); \
} while (0)

#define TRACE MUGGLE_LOG_LEVEL_TRACE
#define DEBUG MUGGLE_LOG_LEVEL_DEBUG
#define INFO MUGGLE_LOG_LEVEL_INFO
#define WARN MUGGLE_LOG_LEVEL_WARN
#define ERROR MUGGLE_LOG_LEVEL_ERROR
#define FATAL MUGGLE_LOG_LEVEL_FATAL

void example_customize_log()
{
	// init customize logger
	muggle_logger_t *logger = customize_async_logger();
	muggle_async_logger_init((muggle_async_logger_t*)logger, 4096);
	example_logger_add_handler(logger);

	MY_LOG(TRACE, "customize logger - trace");
	MY_LOG(DEBUG, "customize logger - debug");
	MY_LOG(INFO, "customize logger - info");
	MY_LOG(WARN, "customize logger - warning");
	MY_LOG(ERROR, "customize logger - error");

	MY_LOG(ERROR, "fatal level will print stack and core dumped when debug");
	MY_LOG(FATAL, "fatal");
}

int main()
{
	example_simple_log_fmt();
	example_complicated_log_fmt();
	example_customize_log_fmt();

	example_log_console_handler();
	example_log_file_handler();
	example_log_file_rotate_handler();
	example_log_file_time_rot_handler();

	example_sync_logger();
	example_async_logger();

	example_simple_log();
	example_customize_log();

	return 0;
}
