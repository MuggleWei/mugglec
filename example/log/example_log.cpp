#include "muggle/c/muggle_c.h"

void example_log_handle_console(int write_type, int fmt_flag, int level)
{
	muggle_log_handle_t handle;
	muggle_log_handle_console_init(
		&handle,
		write_type,
		fmt_flag,
		level,
		0, 1);

	muggle_log_fmt_arg_t arg = {
		MUGGLE_LOG_LEVEL_INFO, __LINE__, __FILE__, __FUNCTION__
	};
	muggle_log_handle_write(&handle, &arg, "console logging");
	muggle_log_handle_write(&handle, &arg, "message info");
	arg.level = MUGGLE_LOG_LEVEL_WARNING;
	muggle_log_handle_write(&handle, &arg, "message warning");
	arg.level = MUGGLE_LOG_LEVEL_ERROR;
	muggle_log_handle_write(&handle, &arg, "message error");

	muggle_log_handle_destroy(&handle);
}

void example_log_handle_file(int write_type, int fmt_flag, int level, const char *path)
{
	muggle_log_handle_t handle;
	int ret = muggle_log_handle_file_init(
		&handle,
		write_type,
		fmt_flag,
		level,
		0, path
	);
	if (ret != MUGGLE_OK)
	{
		fprintf(stderr, "failed init file log handle\n");
		return;
	}

	muggle_log_fmt_arg_t arg = {
		MUGGLE_LOG_LEVEL_INFO, __LINE__, __FILE__, __FUNCTION__
	};
	muggle_log_handle_write(&handle, &arg, "message info");
	arg.level = MUGGLE_LOG_LEVEL_WARNING;
	muggle_log_handle_write(&handle, &arg, "message warning");
	arg.level = MUGGLE_LOG_LEVEL_ERROR;
	muggle_log_handle_write(&handle, &arg, "message error");

	arg.level = MUGGLE_LOG_LEVEL_INFO;
	char buf[1024];
	for (int i = 0; i < 100; i++)
	{
		snprintf(buf, 1024, "[write type: %d] file logging: %d", write_type, i);
		muggle_log_handle_write(&handle, &arg, buf);
	}

	muggle_log_handle_destroy(&handle);
}

void example_log_handle_rotating_file(int write_type, int fmt_flag, int level, const char *path)
{
	muggle_log_handle_t handle;
	int ret = muggle_log_handle_rotating_file_init(
		&handle,
		write_type,
		fmt_flag,
		level,
		0, path,
		1024 * 10, 5
	);
	if (ret != MUGGLE_OK)
	{
		fprintf(stderr, "failed init file log handle\n");
		return;
	}

	muggle_log_fmt_arg_t arg = {
		MUGGLE_LOG_LEVEL_INFO, __LINE__, __FILE__, __FUNCTION__
	};
	muggle_log_handle_write(&handle, &arg, "message info");
	arg.level = MUGGLE_LOG_LEVEL_WARNING;
	muggle_log_handle_write(&handle, &arg, "message warning");
	arg.level = MUGGLE_LOG_LEVEL_ERROR;
	muggle_log_handle_write(&handle, &arg, "message error");

	arg.level = MUGGLE_LOG_LEVEL_INFO;
	char buf[1024];
	for (int i = 0; i < 100; i++)
	{
		snprintf(buf, 1024, "[write type: %d] rotating file logging: %d", write_type, i);
		muggle_log_handle_write(&handle, &arg, buf);
	}

	muggle_log_handle_destroy(&handle);
}

int main()
{
	int fmt = MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_TIME;
	int level = MUGGLE_LOG_LEVEL_INFO;
	char buf[MUGGLE_MAX_PATH];

	// console
	for (int i = 0; i < MUGGLE_LOG_WRITE_TYPE_MAX; ++i)
	{
		example_log_handle_console(i, fmt, level);
	}

	// file
	const char *log_file_path = "log/file/example.log";
	muggle_path_dirname(log_file_path, buf, sizeof(buf));
	if (!muggle_path_exists(buf))
	{
		muggle_os_mkdir(buf);
	}

	for (int i = 0; i < MUGGLE_LOG_WRITE_TYPE_MAX; ++i)
	{
		example_log_handle_file(i, fmt, level, log_file_path);
	}

	// rotating file
	const char *log_rotating_file_path = "log/rotating_file/example.log";
	muggle_path_dirname(log_rotating_file_path, buf, sizeof(buf));
	if (!muggle_path_exists(buf))
	{
		muggle_os_mkdir(buf);
	}

	for (int i = 0; i < MUGGLE_LOG_WRITE_TYPE_MAX; ++i)
	{
		example_log_handle_rotating_file(i, fmt, level, log_rotating_file_path);
	}

	return 0;
}
