/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef __MUGGLE_LOG_H__
#define __MUGGLE_LOG_H__

#include "muggle/base/macro.h"

// log
#define MUGGLE_INFO(format, ...) LogFunction(format, ##__VA_ARGS__)
#define MUGGLE_WARNING(format, ...) \
do \
{ \
	if (!(0)) \
	{ \
		ExportWarning("MUGGLE WARNING", __FILE__, __LINE__, format, ##__VA_ARGS__);  \
	} \
} while(0)
#define MUGGLE_ERROR(format, ...) \
do \
{ \
	if (!(0)) \
	{ \
		ExportFailure("MUGGLE ERROR", __FILE__, __LINE__, format, ##__VA_ARGS__);  \
	} \
} while(0)

// assert
#if MUGGLE_RELEASE
#define MUGGLE_ASSERT(x)
#define MUGGLE_ASSERT_MSG(x, format, ...)
#define MUGGLE_DEBUG_INFO(format, ...)
#define MUGGLE_DEBUG_WARNING(format, ...)
#define MUGGLE_DEBUG_ERROR(format, ...)
#else
#define MUGGLE_ASSERT(x) \
do \
{ \
	if (!(x)) \
	{ \
		ExportFailure(#x, __FILE__, __LINE__, "\0");  \
	} \
} while(0)
#define MUGGLE_ASSERT_MSG(x, format, ...) \
do \
{ \
	if (!(x)) \
	{ \
		ExportFailure(#x, __FILE__, __LINE__, format, ##__VA_ARGS__);  \
	} \
} while(0)
#define MUGGLE_DEBUG_INFO(format, ...) MUGGLE_INFO(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_WARNING(format, ...) MUGGLE_WARNING(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_ERROR(format, ...) MUGGLE_ERROR(format, ##__VA_ARGS__)
#endif

EXTERN_C_BEGIN

MUGGLE_BASE_EXPORT void LogFunction(const char *format, ...);
MUGGLE_BASE_EXPORT void ExportWarning(const char* cond, const char* file_name, int line, const char* format, ...);
MUGGLE_BASE_EXPORT void ExportFailure(const char* cond, const char* file_name, int line, const char* format, ...);

EXTERN_C_END

#endif