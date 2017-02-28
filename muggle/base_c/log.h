/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef __MUGGLE_LOG_H__
#define __MUGGLE_LOG_H__

#include <time.h>
#include "muggle/base_c/macro.h"
#include "muggle/base_c/mutex.h"

#define MUGGLE_MAX_LOG_LEN 2048

#define MUGGLE_LOG_DEFAULT(level, format, ...) \
do \
{ \
	if (!(0)) \
	{ \
		LogAttribute attr = { \
			level, __LINE__, __FILE__, \
			__FUNCTION__, 0 \
		}; \
		LogDefault(&attr, format, ##__VA_ARGS__); \
	} \
} while (0)

#define MUGGLE_LOG(ptr_log_handle, level, format, ...) \
do \
{ \
	if (!(0)) \
	{ \
		LogAttribute attr = { \
			level, __LINE__, __FILE__, \
			__FUNCTION__, 0 \
		}; \
		LogFunction(ptr_log_handle, &attr, format, ##__VA_ARGS__); \
	} \
} while (0)

#define MUGGLE_INFO(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define MUGGLE_WARNING(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define MUGGLE_ERROR(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_ERROR, format, ##__VA_ARGS__)

#if MUGGLE_RELEASE
#define MUGGLE_ASSERT(x)
#define MUGGLE_ASSERT_MSG(x, format, ...)
#define MUGGLE_DEBUG_INFO(format, ...)
#define MUGGLE_DEBUG_WARNING(format, ...)
#define MUGGLE_DEBUG_ERROR(format, ...)
#define MUGGLE_DEBUG_LOG(ptr_log_handle, level, format, ...)
#else
#define MUGGLE_ASSERT(x) \
do \
{ \
	if (!(x)) \
	{ \
		LogAttribute attr = { \
			MUGGLE_LOG_LEVEL_ERROR, __LINE__, __FILE__, \
			__FUNCTION__, 0 \
		}; \
		LogDefault(&attr, "Assertion: "#x); \
	} \
} while (0)
#define MUGGLE_ASSERT_MSG(x, format, ...) \
do \
{ \
	if (!(x)) \
	{ \
		LogAttribute attr = { \
			MUGGLE_LOG_LEVEL_ERROR, __LINE__, __FILE__, \
			__FUNCTION__, 0 \
		}; \
		LogDefault(&attr, "Assertion: "#x format, ##__VA_ARGS__); \
	} \
} while (0)
#define MUGGLE_DEBUG_INFO(format, ...) MUGGLE_INFO(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_WARNING(format, ...) MUGGLE_WARNING(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_ERROR(format, ...) MUGGLE_ERROR(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_LOG(ptr_log_handle, level, format, ...) MUGGLE_LOG(ptr_log_handle, level, format, ##__VA_ARGS__)
#endif

EXTERN_C_BEGIN

enum eMuggleLogLevel
{
	MUGGLE_LOG_LEVEL_OFFSET = 8,
	MUGGLE_LOG_LEVEL_INFO = 1 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_WARNING = 2 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_ERROR = 3 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_MAX = 4,
};

enum eMuggleLogFormat
{
	MUGGLE_LOG_FMT_LEVEL = 0x01,
	MUGGLE_LOG_FMT_FILE = 0x02,
	MUGGLE_LOG_FMT_LINE = 0x04,
	MUGGLE_LOG_FMT_FUNC = 0x08,
	MUGGLE_LOG_FMT_TIME = 0x20,
};

enum eMuggleLogDefault
{
	MUGGLE_LOG_DEFAULT_CONSOLE = 0,
	MUGGLE_LOG_DEFAULT_FILE,
	MUGGLE_LOG_DEFAULT_WIN_DEBUG_OUT,
	MUGGLE_LOG_DEFAULT_MAX,
};
enum eMuggleLogDefaultType
{
	MUGGLE_LOG_DEFAULT_TYPE_CONSOLE = 1 << MUGGLE_LOG_DEFAULT_CONSOLE,
	MUGGLE_LOG_DEFAULT_TYPE_FILE = 1 << MUGGLE_LOG_DEFAULT_FILE,
	MUGGLE_LOG_DEFAULT_TYPE_WIN_DEBUG_OUT = 1 << MUGGLE_LOG_DEFAULT_WIN_DEBUG_OUT,
	MUGGLE_LOG_DEFAULT_ALL = 
		MUGGLE_LOG_DEFAULT_TYPE_CONSOLE | 
		MUGGLE_LOG_DEFAULT_TYPE_FILE | 
		MUGGLE_LOG_DEFAULT_TYPE_WIN_DEBUG_OUT,
};

struct LogHandle_tag;
struct LogAttribute_tag;

typedef void(*ptrLogFunc)(
	struct LogHandle_tag *log_handle,
	struct LogAttribute_tag *attr,
	const char *msg);

typedef struct LogHandle_tag
{
	void *io_target;
	ptrLogFunc func;
	MutexHandle *mtx;
	int format;
}LogHandle;

typedef struct LogAttribute_tag
{
	int level;
	int line;
	const char *file;
	const char *func;
	time_t time;
}LogAttribute;

/*
 *	generate format log text
 *	@msg: input message
 *	@buf: buffer for store format text
 *	@max_len: the max number of bytes that buffer can store
 *	RETURN: number of bytes in buffer
 */
MUGGLE_BASE_C_EXPORT int LogGenFmtText(LogHandle *log_handle, LogAttribute *attr, const char *msg, char *buf, int max_len);
/*
 *	initialize default logs, without this, output log in console by default.
 *	note: don't invoke this more than once
 *	@fmt: log output format, operator or in eMuggleLogFormat
 *	@log_file_path: the file path for log output
 *	@enable_console_color: whether or not enable colored print in console
 */
MUGGLE_BASE_C_EXPORT void LogDefaultInit(const char *log_file_path, int enable_console_color);
/*
 *	switch default log output, before invoke this
 *	NOTE: ensure LogDefaultInit was invoked if you want invoke default log output function
 *	@log_idx: log index (in eMuggleLogDefault)
 *	@log_handle: replace default log handle, if this is not NULL, the latter two parameters are ignored
 *	@enable: active or inactive this default output
 *	@fmt: reset the format of this default output (operator or in eMuggleLogFormat, 0 represent only output message text)
 */
MUGGLE_BASE_C_EXPORT void LogDefaultSwitch(int log_idx, LogHandle *log_handle, int enable, int fmt);
/*
 *	the two function below just for macro invoke, don't use them directly
 */
MUGGLE_BASE_C_EXPORT void LogDefault(LogAttribute *attr, const char *format, ...);
MUGGLE_BASE_C_EXPORT void LogFunction(LogHandle *log_handle, LogAttribute *attr, const char *format, ...);

EXTERN_C_END

#endif