#include "muggle/c/muggle_c.h"

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
		GEN_TMP_LOG_MSG(msg, "example log file handler, write into log/example_log.log");

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

void example_simple_log()
{
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_TRACE);

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

	MUGGLE_LOG_ERROR("fatal level will print stack and core dumped when debug");
	MUGGLE_LOG_FATAL("fatal");
}

int main()
{
	example_simple_log_fmt();
	example_complicated_log_fmt();
	example_customize_log_fmt();

	example_log_console_handler();
	example_log_file_handler();
	example_log_file_rotate_handler();

	example_simple_log();

	return 0;
}
