#include "muggle/c/muggle_c.h"

void example_log_handle_console(int write_type, int fmt_flag, int level)
{
	muggle_log_handle_t handle;
	muggle_log_handle_console_init(
		&handle, write_type, fmt_flag, level, 0, NULL, NULL, 1);

	muggle_log_fmt_arg_t arg = {
		MUGGLE_LOG_LEVEL_INFO, __LINE__, __FILE__, __FUNCTION__, muggle_thread_current_id()
	};
	muggle_log_handle_write(&handle, &arg, "console logging");
	arg.level = MUGGLE_LOG_LEVEL_TRACE;
	muggle_log_handle_write(&handle, &arg, "message trace");
	arg.level = MUGGLE_LOG_LEVEL_INFO;
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
		&handle, write_type, fmt_flag, level,	0, NULL, NULL, path);
	if (ret != MUGGLE_OK)
	{
		fprintf(stderr, "failed init file log handle\n");
		return;
	}

	muggle_log_fmt_arg_t arg = {
		MUGGLE_LOG_LEVEL_TRACE, __LINE__, __FILE__, __FUNCTION__, muggle_thread_current_id()
	};
	muggle_log_handle_write(&handle, &arg, "message trace");
	arg.level = MUGGLE_LOG_LEVEL_INFO;
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
		&handle, write_type, fmt_flag, level, 0, NULL, NULL,
		path, 1024 * 10, 5
	);
	if (ret != MUGGLE_OK)
	{
		fprintf(stderr, "failed init file log handle\n");
		return;
	}

	muggle_log_fmt_arg_t arg = {
		MUGGLE_LOG_LEVEL_TRACE, __LINE__, __FILE__, __FUNCTION__, muggle_thread_current_id()
	};
	muggle_log_handle_write(&handle, &arg, "message trace");
	arg.level = MUGGLE_LOG_LEVEL_INFO;
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

void example_log_handle_win_debug(int write_type, int fmt_flag, int level)
{
	muggle_log_handle_t handle;
	muggle_log_handle_win_debug_init(
		&handle, write_type, fmt_flag, level, 0, NULL, NULL);

	muggle_log_fmt_arg_t arg = {
		MUGGLE_LOG_LEVEL_INFO, __LINE__, __FILE__, __FUNCTION__, muggle_thread_current_id()
	};
	muggle_log_handle_write(&handle, &arg, "win debug logging");
	arg.level = MUGGLE_LOG_LEVEL_TRACE;
	muggle_log_handle_write(&handle, &arg, "message trace");
	arg.level = MUGGLE_LOG_LEVEL_INFO;
	muggle_log_handle_write(&handle, &arg, "message info");
	arg.level = MUGGLE_LOG_LEVEL_WARNING;
	muggle_log_handle_write(&handle, &arg, "message warning");
	arg.level = MUGGLE_LOG_LEVEL_ERROR;
	muggle_log_handle_write(&handle, &arg, "message error");

	muggle_log_handle_destroy(&handle);
}

void example_log_category(int write_type, int fmt_flag, int level)
{
	muggle_log_handle_t handle_console;
	muggle_log_handle_console_init(
		&handle_console, write_type, fmt_flag, level, 0, NULL, NULL, 1);


	char buf[MUGGLE_MAX_PATH];
	const char *file_path = "log/category/example.log";
	muggle_path_dirname(file_path, buf, sizeof(buf));
	if (!muggle_path_exists(buf))
	{
		muggle_os_mkdir(buf);
	}

	muggle_log_handle_t handle_file;
	muggle_log_handle_file_init(
		&handle_file, write_type, fmt_flag, level, 0, NULL, NULL, file_path);

	muggle_log_category_t category;
	memset(&category, 0, sizeof(category));

	muggle_log_category_add(&category, &handle_console);
	muggle_log_category_add(&category, &handle_file);

	muggle_log_fmt_arg_t arg = {
		MUGGLE_LOG_LEVEL_INFO, __LINE__, __FILE__, __FUNCTION__, muggle_thread_current_id()
	};
	muggle_log_category_write(&category, &arg, "category");
	arg.level = MUGGLE_LOG_LEVEL_TRACE;
	muggle_log_category_write(&category, &arg, "category message trace");
	arg.level = MUGGLE_LOG_LEVEL_INFO;
	muggle_log_category_write(&category, &arg, "category message info");
	arg.level = MUGGLE_LOG_LEVEL_WARNING;
	muggle_log_category_write(&category, &arg, "category message warning");
	arg.level = MUGGLE_LOG_LEVEL_ERROR;
	muggle_log_category_write(&category, &arg, "category message error");

	muggle_log_category_destroy(&category, 1);
}

muggle_log_handle_t handle_console;
muggle_log_handle_t handle_rotating_file;
void init_log()
{
	muggle_log_handle_console_init(
		&handle_console,
		MUGGLE_LOG_WRITE_TYPE_SYNC,
		MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_THREAD,
		MUGGLE_LOG_LEVEL_WARNING,
		0, NULL, NULL, 1);
	muggle_log_add_handle(&handle_console);

	char buf[MUGGLE_MAX_PATH];
	const char *file_path = "log/default/example.log";
	muggle_path_dirname(file_path, buf, sizeof(buf));
	if (!muggle_path_exists(buf))
	{
		muggle_os_mkdir(buf);
	}
	int ret = muggle_log_handle_rotating_file_init(
		&handle_rotating_file,
		MUGGLE_LOG_WRITE_TYPE_SYNC,
		MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_TIME | MUGGLE_LOG_FMT_THREAD,
		MUGGLE_LOG_LEVEL_TRACE,
		0, NULL, NULL, file_path,
		1024 * 10, 5
	);
	if (ret != MUGGLE_OK)
	{
		MUGGLE_LOG_ERROR("failed init log rotating file handle with path: %s", file_path);
		return;
	}
	muggle_log_add_handle(&handle_rotating_file);
}

void destroy_log()
{
	muggle_log_destroy();
}

muggle_thread_ret_t example_thread_output_idx(void *void_arg)
{
	int idx = (int)(intptr_t)void_arg;
	MUGGLE_LOG_INFO("info: %d", idx);
	return 0;
}

void example_default()
{
	init_log();

	muggle_thread_t threads[8];
	for (int i = 0; i < 8; ++i)
	{
		muggle_thread_create(&threads[i], example_thread_output_idx, (void*)(intptr_t)i);
	}

	MUGGLE_DEBUG_TRACE("debug trace");
	MUGGLE_DEBUG_INFO("debug info");
	MUGGLE_DEBUG_WARNING("debug warning");
	MUGGLE_ASSERT_MSG(1 == 1, "assert message");

	MUGGLE_LOG_TRACE("trace");
	MUGGLE_LOG_INFO("info");
	MUGGLE_LOG_WARNING("warning");
	MUGGLE_LOG_ERROR("error");
	MUGGLE_LOG_FATAL("fatal, core dump when debug");

	for (int i = 0; i < 8; ++i)
	{
		muggle_thread_join(&threads[i]);
	}

	destroy_log();
}

int main()
{
	// default (recommend)
	example_default();

	int fmt = MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_TIME | MUGGLE_LOG_FMT_THREAD;
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

	// win debug
	for (int i = 0; i < MUGGLE_LOG_WRITE_TYPE_MAX; ++i)
	{
		example_log_handle_win_debug(i, fmt, level);
	}

	// category
	for (int i = 0; i < MUGGLE_LOG_WRITE_TYPE_MAX; ++i)
	{
		example_log_category(i, fmt, level);
	}

	return 0;
}
