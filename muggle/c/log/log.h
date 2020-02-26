/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_LOG_H_
#define MUGGLE_C_LOG_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/log/log_fmt.h"
#include "muggle/c/log/log_handle.h"
#include "muggle/c/log/log_category.h"

EXTERN_C_BEGIN

#define MUGGLE_LOG_DEFAULT(level, format, ...) \
do \
{ \
	muggle_log_fmt_arg_t mlf_arg##__LINE__ = { \
		level, __LINE__, __FILE__, __FUNCTION__, muggle_thread_current_id() \
	}; \
	muggle_log_function(&g_log_default_category, &mlf_arg##__LINE__, format, ##__VA_ARGS__); \
} while (0)

#define MUGGLE_LOG(p_log_category, level, format, ...) \
do \
{ \
	muggle_log_fmt_arg_t mlf_arg##__LINE__ = { \
		level, __LINE__, __FILE__, __FUNCTION__, muggle_thread_current_id() \
	}; \
	muggle_log_function(p_log_category, &mlf_arg##__LINE__, format, ##__VA_ARGS__); \
} while (0)

#define MUGGLE_TRACE(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_TRACE, format, ##__VA_ARGS__)
#define MUGGLE_INFO(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define MUGGLE_WARNING(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define MUGGLE_ERROR(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define MUGGLE_FATAL(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_FATAL, format, ##__VA_ARGS__)


#if MUGGLE_RELEASE

#define MUGGLE_ASSERT(x)
#define MUGGLE_ASSERT_MSG(x, format, ...)
#define MUGGLE_DEBUG_TRACE(format, ...)
#define MUGGLE_DEBUG_INFO(format, ...)
#define MUGGLE_DEBUG_WARNING(format, ...)
#define MUGGLE_DEBUG_ERROR(format, ...)
#define MUGGLE_DEBUG_FATAL(format, ...)
#define MUGGLE_DEBUG_LOG(ptr_log_handle, level, format, ...)

#else

#define MUGGLE_ASSERT(x) \
do \
{ \
	if (!(x)) \
	{ \
		muggle_log_fmt_arg_t mlf_arg##__LINE__ = { \
			MUGGLE_LOG_LEVEL_FATAL, __LINE__, __FILE__, __FUNCTION__, muggle_thread_current_id() \
		}; \
		muggle_log_function(&g_log_default_category, &mlf_arg##__LINE__, "Assertion: "#x); \
	} \
} while (0)

#define MUGGLE_ASSERT_MSG(x, format, ...) \
do \
{ \
	if (!(x)) \
	{ \
		muggle_log_fmt_arg_t mlf_arg##__LINE__ = { \
			MUGGLE_LOG_LEVEL_FATAL, __LINE__, __FILE__, __FUNCTION__, muggle_thread_current_id() \
		}; \
		muggle_log_function(&g_log_default_category, &mlf_arg##__LINE__, "Assertion: "#x format, ##__VA_ARGS__); \
	} \
} while (0)

#define MUGGLE_DEBUG_TRACE(format, ...) MUGGLE_TRACE(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_INFO(format, ...) MUGGLE_INFO(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_WARNING(format, ...) MUGGLE_WARNING(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_ERROR(format, ...) MUGGLE_ERROR(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_FATAL(format, ...) MUGGLE_FATAL(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_LOG(ptr_log_handle, level, format, ...) MUGGLE_LOG(ptr_log_handle, level, format, ##__VA_ARGS__)

#endif

MUGGLE_CC_EXPORT
extern muggle_log_category_t g_log_default_category;

/*
 * add log handle to default log category
 * */
MUGGLE_CC_EXPORT
void muggle_log_add_handle(muggle_log_handle_t *handle);

/*
 * destroy default log category
 * */
MUGGLE_CC_EXPORT
void muggle_log_destroy();

/*
 *	output log, don't use this function immediately, use MUGGLE_LOG macro instead
 *	@category: the category need to output
 *	@arg: log attribute information
 *	@format: like print(format, ...);
 */
MUGGLE_CC_EXPORT
void muggle_log_function(
	muggle_log_category_t *category,
	muggle_log_fmt_arg_t *arg,
	const char *format,
	...);

EXTERN_C_END

#endif
