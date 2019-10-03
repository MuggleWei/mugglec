#include "muggle/c/muggle_c.h"

void example_log_handle_console(int write_type, int fmt_flag, muggle_atomic_int async_capacity)
{
	muggle_log_handle_t handle;
	muggle_log_handle_console_init(
		&handle,
		write_type,
		fmt_flag,
		async_capacity,
		1);

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

int main()
{
	for (int i = 0; i < MUGGLE_LOG_WRITE_TYPE_MAX; ++i)
	{
	example_log_handle_console(
		i,
		MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_TIME,
		0);
	}

	return 0;
}
