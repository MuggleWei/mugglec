#include "muggle/c/muggle_c.h"

muggle_logger_t* my_async_logger()
{
	static muggle_async_logger_t async_logger;
	return (muggle_logger_t*)&async_logger;
}

void init_log()
{
	muggle_logger_t *logger = my_async_logger();
	muggle_async_logger_init((muggle_async_logger_t*)logger, 4096);

	static muggle_log_console_handler_t console_handler;
	muggle_log_console_handler_init(&console_handler, 1);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&console_handler, LOG_LEVEL_DEBUG);

	logger->add_handler(logger, (muggle_log_handler_t*)&console_handler);
}

void destroy_log()
{
	muggle_logger_t *logger = my_async_logger();
	logger->destroy(logger);
}

int main()
{
	init_log();

	MUGGLE_LOG(my_async_logger(), LOG_LEVEL_INFO, "hello async logger");

	destroy_log();

	return 0;
}
