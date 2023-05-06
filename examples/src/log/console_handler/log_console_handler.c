#include "muggle/c/muggle_c.h"

void init_log()
{
	static muggle_log_console_handler_t console_handler;
	muggle_log_console_handler_init(&console_handler, 1);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&console_handler, LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&console_handler);
}

int main()
{
	init_log();
	
	LOG_INFO("info message");
	LOG_WARNING("warning message");
	LOG_ERROR("error message");
	LOG_INFO("bye");

	return 0;
}
