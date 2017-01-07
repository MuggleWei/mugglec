/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif

void LogFunction(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	char buf[MUGGLE_MAX_PATH];
	vsnprintf(buf, MUGGLE_MAX_PATH, format, args);

#if MUGGLE_PLATFORM_WINDOWS
	WCHAR w_buf[MUGGLE_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, w_buf, MUGGLE_MAX_PATH);
	OutputDebugStringW(w_buf);
	WideCharToMultiByte(CP_ACP, 0, w_buf, -1, buf, MUGGLE_MAX_PATH, NULL, FALSE);
#endif

	fprintf(stdout, "%s", buf);

	va_end(args);
}

void ExportWarning(const char* cond, const char* file_name, int line, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buf[MUGGLE_MAX_PATH];
	vsnprintf(buf, MUGGLE_MAX_PATH, format, args);

#if MUGGLE_PLATFORM_WINDOWS
	WCHAR w_buf[MUGGLE_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, w_buf, MUGGLE_MAX_PATH);
	OutputDebugStringW(w_buf);
	WideCharToMultiByte(CP_ACP, 0, w_buf, -1, buf, MUGGLE_MAX_PATH, NULL, FALSE);
#endif

	fprintf(stderr, "[%s: %d] Warning: %s. %s\n", file_name, line, cond, buf);

	va_end(args);
}

void ExportFailure(const char* cond, const char* file_name, int line, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buf[MUGGLE_MAX_PATH];
	vsnprintf(buf, MUGGLE_MAX_PATH, format, args);

#if MUGGLE_PLATFORM_WINDOWS
	WCHAR w_buf[MUGGLE_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, w_buf, MUGGLE_MAX_PATH);
	OutputDebugStringW(w_buf);
	WideCharToMultiByte(CP_ACP, 0, w_buf, -1, buf, MUGGLE_MAX_PATH, NULL, FALSE);
#endif
	
	fprintf(stderr, "[%s: %d] Assertion failed: %s. %s\n", file_name, line, cond, buf);

#if MUGGLE_DEBUG

#if MUGGLE_PLATFORM_WINDOWS
	__debugbreak();
#endif

	abort();
#endif

	va_end(args);
}