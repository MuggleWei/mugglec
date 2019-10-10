/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "log_handle_win_debug.h"
#include <stdio.h>
#include <string.h>
#include "muggle/c/base/err.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#endif

int muggle_log_handle_win_debug_init(
	muggle_log_handle_t *handle,
	int write_type,
	int fmt_flag,
	int level,
	muggle_atomic_int async_capacity,
	muggle_log_handle_async_alloc p_alloc,
	muggle_log_handle_async_free p_free
)
{
	handle->type = MUGGLE_LOG_TYPE_WIN_DEBUG_OUT;
	int ret = muggle_log_handle_base_init(handle, write_type, fmt_flag, level, async_capacity, p_alloc, p_free);
	if (ret != MUGGLE_OK)
	{
		return ret;
	}

	return MUGGLE_OK;
}

int muggle_log_handle_win_debug_destroy(muggle_log_handle_t *handle)
{
	return MUGGLE_OK;
}

int muggle_log_handle_win_debug_output(
	muggle_log_handle_t *handle,
	muggle_log_fmt_arg_t *arg,
	const char *msg
)
{
#if MUGGLE_PLATFORM_WINDOWS
	if (!IsDebuggerPresent())
	{
		return 0;
	}

	int ret;
	char buf[MUGGLE_LOG_MAX_LEN] = { 0 };

	ret = muggle_log_fmt_gen(handle->fmt_flag, arg, msg, buf, sizeof(buf));
	if (ret <= 0)
	{
		return ret;
	}

	WCHAR w_buf[MUGGLE_LOG_MAX_LEN];
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, w_buf, sizeof(w_buf));

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_lock(&handle->sync.mutex);
	}

	OutputDebugStringW(w_buf);

	if (handle->write_type == MUGGLE_LOG_WRITE_TYPE_SYNC)
	{
		muggle_mutex_unlock(&handle->sync.mutex);
	}


	return ret;
#else
	return 0;
#endif
}
