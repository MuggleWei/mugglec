#include "muggle/c/muggle_c.h"

void init_log()
{
	static muggle_log_file_time_rot_handler_t file_time_rot_handler;
	muggle_log_file_time_rot_handler_init(
		&file_time_rot_handler, "log/example_time_rot.log",
		MUGGLE_LOG_TIME_ROTATE_UNIT_DAY, 1, false);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_time_rot_handler, LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_time_rot_handler);
}

int main()
{
	init_log();

	MUGGLE_LOG_INFO("default logger with file time rotate handler");

	return 0;
}