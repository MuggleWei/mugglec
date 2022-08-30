#include "muggle/c/muggle_c.h"

void init_log()
{
	static muggle_log_file_rotate_handler_t file_rot_handler;
	muggle_log_file_rotate_handler_init(
		&file_rot_handler, "log/example_rot.log",
		16 * 1024, 5);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_rot_handler, LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_rot_handler);
}

int main()
{
	init_log();
	
	for (int i = 0; i < 4096; i++)
	{
		MUGGLE_LOG_INFO("default logger with file rotate handler, #%d", i);
	}

	return 0;
}