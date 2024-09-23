#include "muggle/c/muggle_c.h"

void init_log(const char *filepath)
{
	static muggle_log_file_time_rot_handler_t file_time_rot_handler;

	int ret = muggle_log_file_time_rot_handler_init(
		&file_time_rot_handler, filepath,
		MUGGLE_LOG_TIME_ROTATE_UNIT_DAY, 1, false);
	if (ret != 0) {
		fprintf(stderr, "failed open file: %s\n", filepath);
		return;
	}
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_time_rot_handler, LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_time_rot_handler);
}

int main(int argc, char *argv[])
{
	const char *filepath = "logs/example_time_rot.log";
	if (argc > 1) {
		filepath = argv[1];
	}

	init_log(filepath);

	MUGGLE_LOG_INFO("default logger with file time rotate handler");

	return 0;
}
