/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_LOG_H_
#define MUGGLE_C_LOG_H_

#include <time.h>
#include "muggle/base_c/macro.h"

#define MUGGLE_LOG_DEFAULT(level, format, ...) \
do \
{ \
	MuggleLogAttributeInfo attr = { \
		level, __LINE__, __FILE__, \
		__FUNCTION__ \
	}; \
	MuggleLogFunction(&g_log_default_category, &attr, format, ##__VA_ARGS__); \
} while (0)

#define MUGGLE_LOG(p_log_category, level, format, ...) \
do \
{ \
	MuggleLogAttributeInfo attr = { \
		level, __LINE__, __FILE__, \
		__FUNCTION__ \
	}; \
	MuggleLogFunction(p_log_category, &attr, format, ##__VA_ARGS__); \
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
		MuggleLogAttributeInfo attr = { \
			MUGGLE_LOG_LEVEL_ERROR, __LINE__, __FILE__, \
			__FUNCTION__ \
		}; \
		MuggleLogFunction(&g_log_default_category, &attr, "Assertion: "#x); \
	} \
} while (0)
#define MUGGLE_ASSERT_MSG(x, format, ...) \
do \
{ \
	if (!(x)) \
	{ \
		MuggleLogAttributeInfo attr = { \
			MUGGLE_LOG_LEVEL_ERROR, __LINE__, __FILE__, \
			__FUNCTION__ \
		}; \
		MuggleLogFunction(&g_log_default_category, &attr, "Assertion: "#x format, ##__VA_ARGS__); \
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

enum eMuggleLogErrorCode
{
	MUGGLE_LOG_ERROR_CODE_NULL_CATEGORY = 1,
	MUGGLE_LOG_ERROR_CODE_NULL_HANDLE,
	MUGGLE_LOG_ERROR_CODE_NULL_LOG_FUNC,
	MUGGLE_LOG_ERROR_CODE_NULL_ATTRIBUTE,
	MUGGLE_LOG_ERROR_CODE_HANDLE_REPEAT_MOUNT,
	MUGGLE_LOG_ERROR_CODE_ALLOC_MEMORY,
	MUGGLE_LOG_ERROR_CODE_COULDNOT_FOUND,
};

enum
{
	MUGGLE_LOG_MAX_LEN = 2048,
	MUGGLE_LOG_MAX_NAME_LEN = 31,
	MUGGLE_LOG_HANDLE_RESERVE_SIZE = 64,
};

struct MuggleLogHandle_tag;
struct MuggleLogAttributeInfo_tag;

/*
 *	pointer that point to log output function
 *	RETURN: if failed, return negative value
 */
typedef int (*MugglePtrLogFunc)(
	struct MuggleLogHandle_tag *log_handle,
	struct MuggleLogAttributeInfo_tag *attr,
	const char *msg);

/*
 *	pointer that point to log format function
 *	RETURN: len of buffer use
 */
typedef int (*MugglePtrLogFormatFunc)(
	struct MuggleLogHandle_tag *log_handle,
	struct MuggleLogAttributeInfo_tag *attr,
	const char *msg,
	char *buf,
	int max_len);

/*
 *	pointer that point to log handle initialize function
 *	RETURN: if failed, return negative value
 */
typedef int (*MugglePtrLogInitFunc)(
	struct MuggleLogHandle_tag *log_handle);

/*
 *	pointer that point to log handle destroy function
 *	RETURN: if failed, return negative value
 */
typedef int (*MugglePtrLogDestroyFunc)(
	struct MuggleLogHandle_tag *log_handle);

#define MUGGLE_LOG_HANDLE_BASE_STRUCT \
struct MuggleLogHandle_tag *next; \
unsigned int flags; \
MugglePtrLogFunc log_func; \
MugglePtrLogFormatFunc format_func; \
MugglePtrLogDestroyFunc destroy_func; \
char name[MUGGLE_LOG_MAX_NAME_LEN + 1]; \

typedef struct MuggleLogHandle_tag
{
	MUGGLE_LOG_HANDLE_BASE_STRUCT
	unsigned char reserve[MUGGLE_LOG_HANDLE_RESERVE_SIZE];
}MuggleLogHandle;

typedef struct MuggleLogAttributeInfo_tag
{
	unsigned int level;
	int line;
	const char *file;
	const char *func;
}MuggleLogAttributeInfo;

typedef struct MuggleLogCategory_tag
{
	char name[MUGGLE_LOG_MAX_NAME_LEN + 1];
	unsigned int priority;
	MuggleLogHandle *head;
}MuggleLogCategory;

// don't modify g_log_default_category immediately, use MuggleLogDefault* function instead 
MUGGLE_BASE_C_EXPORT
extern MuggleLogCategory g_log_default_category;

/*
 *	Generate a muggle log handle
 *	@name: log handle's name
 *	@flags: custom flags
 *	@size: size of Log handle, size = size > sizeof(MuggleLogHandle) ? size : sizeof(MuggleLogHandle);
 *	@log_func: log function
 *	@format_func: format message function
 *	@destroy_func: the function used to destroy data(e.g. file descriptor) in this log
 *		handle, and free memory
 *	RETURN: return genarated log handle, if failed, return NULL
 */
MUGGLE_BASE_C_EXPORT
MuggleLogHandle* MuggleLogGenHandle(
	const char *name,
	unsigned int flags,
	size_t size,
	MugglePtrLogFunc log_func,
	MugglePtrLogFormatFunc format_func,
	MugglePtrLogInitFunc init_func,
	MugglePtrLogDestroyFunc destroy_func);

/*
 *	Destroy a muggle log handle
 *	@log_handle: the log handle need to be destroyed
 *	RETURN: 0 represent success, otherwise return value in eMuggleLogErrorCode
 */
MUGGLE_BASE_C_EXPORT
int MuggleLogDestroyHandle(MuggleLogHandle *log_handle);

/*
 *	Generate a muggle log category
 *	@name: the name of this category
 *	@priority: min level that can output
 *	RETURN: initialized muggle log category pointer
 */
MUGGLE_BASE_C_EXPORT
MuggleLogCategory* MuggleLogGenCategroy(const char *name, unsigned int priority);

/*
 *	Destroy a muggle log category
 *	@category: log category pointer that need to be cleared
 *	RETURN: 0 represent success, otherwise return error code in eMuggleLogErrorCode
 */
MUGGLE_BASE_C_EXPORT
int MuggleLogClearCategory(MuggleLogCategory *category);

/*
 *	Add a log handle into log category
 *	@log_handle: log handle pointer
 *	RETURN: 0 represent success, otherwise return error code in eMuggleLogErrorCode
 */
MUGGLE_BASE_C_EXPORT
int MuggleLogCategoryAddHandle(MuggleLogCategory *category, MuggleLogHandle *log_handle);

/*
 *	Remove a log handle in log category
 *	@category: the category contain log handle
 *	@log_handle: the log handle need to be remove and destroyed
 *	RETURN: 0 represent success, otherwise return error code in eMuggleLogErrorCode
 */
MUGGLE_BASE_C_EXPORT
int MuggleLogCategoryRemoveHandle(MuggleLogCategory *category, MuggleLogHandle *log_handle);

/*
 *	output log, don't use this function immediately, use MUGGLE_LOG macro instead
 *	@category: the category need to output
 *	@attr: log attribute information
 *	@format: just like print(format, ...);
 */
MUGGLE_BASE_C_EXPORT
void MuggleLogFunction(MuggleLogCategory *category, MuggleLogAttributeInfo *attr, const char *format, ...);

/*
 *	generate formatted log text
 *	@log_handle: log handle
 *	@attr: log attribute
 *	@msg: input message
 *	@buf: buffer for store format text
 *	@max_len: the max number of bytes that buffer can store
 *	RETURN: number of bytes in buffer
 */
MUGGLE_BASE_C_EXPORT
int MuggleLogGenFmtText(MuggleLogHandle *log_handle, MuggleLogAttributeInfo *attr, const char *msg, char *buf, int max_len);


/************************** default log function **************************/
/*
 *	The efficiency of default log hangle is not high, just for write simple code conveniently, 
 *	if you wanna use muggle log in real project, avoid use default category, unless you change
 *	the handles in default category.
 */

/*
 *	initialize default log category, without this, output log in console by default.
 *	NOTE: if this function was invoked, need to invoke MuggleLogDefaultExit() when exit program.
 *	@log_file_path: the file path for g_log_default_handles[MUGGLE_LOG_DEFAULT_FILE]
 *	@enable_console_color: whether or not enable colored print in console
 */
MUGGLE_BASE_C_EXPORT
void MuggleLogDefaultInit(const char *log_file_path, int enable_console_color);

/*
 *	clear muggle default log category
 */
MUGGLE_BASE_C_EXPORT
void MuggleLogDefaultClear();

/*
 *	add default log handle into default log category
 *	@default_log_idx: value in eMuggleLogDefault
 *	@log_file_path: when default_log_idx use file, this argument is the path of file
 */
MUGGLE_BASE_C_EXPORT
int MuggleLogDefaultAdd(int default_log_idx, const char *log_file_path);

/*
 *	close handle in default log category
 *	@default_log_idx: value in eMuggleLogDefault
 */
MUGGLE_BASE_C_EXPORT
void MuggleLogDefaultRemove(int default_log_idx);

/*
 *	set log default format
 */
MUGGLE_BASE_C_EXPORT
void MuggleLogDefaultFlags(int default_log_idx, unsigned int flags);

/*
 *	add log handle into default log category
 */
MUGGLE_BASE_C_EXPORT
void MuggleLogDefaultAddHandle(MuggleLogHandle *log_handle);
/*
 *	remove log handle in default log category
 */
MUGGLE_BASE_C_EXPORT
void MuggleLogDefaultRemoveHandle(MuggleLogHandle *log_handle);


EXTERN_C_END

#endif