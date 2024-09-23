#include "muggle/c/muggle_c.h"

void init_log()
{
	static muggle_log_file_handler_t file_handler;

	const char *filepath = "logs/hello.log";
	// const char *filepath = "/tmp/test_mugglec/logs/hello.log";
	if (muggle_log_file_handler_init(&file_handler, filepath, "w") != 0) {
		fprintf(stderr, "failed open file: %s\n", filepath);
		return;
	}
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_handler, LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_handler);
}

int main()
{
	init_log();
	
	LOG_INFO("info message");
	LOG_WARNING("warning message");
	LOG_ERROR("error message");

	return 0;
}
