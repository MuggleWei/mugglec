#include "muggle/c/muggle_c.h"

void init_log(const char *filepath)
{
	static muggle_log_file_handler_t file_handler;

	if (muggle_log_file_handler_init(&file_handler, filepath, "w") != 0) {
		fprintf(stderr, "failed open file: %s\n", filepath);
		return;
	}
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_handler, LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_handler);
}

int main(int argc, char *argv[])
{
	const char *filepath = "logs/hello.log";
	if (argc > 1) {
		filepath = argv[1];
	}

	init_log(filepath);
	
	LOG_INFO("info message");
	LOG_WARNING("warning message");
	LOG_ERROR("error message");

	return 0;
}
