/******************************************************************************
 *  @file         log.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        function declarations of mugglec log
 *****************************************************************************/
 
#ifndef MUGGLE_C_LOG_H_
#define MUGGLE_C_LOG_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/log/log_logger.h"
#include "muggle/c/log/log_level.h"

EXTERN_C_BEGIN

#define MUGGLE_LOG_DEFAULT(level, format, ...) \
do \
{ \
	muggle_log_src_loc_t loc_arg##__LINE__ = { \
		__FILE__, __LINE__, __FUNCTION__ \
	}; \
	muggle_logger_t *logger = muggle_logger_default(); \
	logger->log(logger, level, &loc_arg##__LINE__, format, ##__VA_ARGS__); \
} while (0)

#define MUGGLE_LOG(logger, level, format, ...) \
do \
{ \
	muggle_log_src_loc_t loc_arg##__LINE__ = { \
		__FILE__, __LINE__, __FUNCTION__ \
	}; \
	logger->log(logger, level, &loc_arg##__LINE__, format, ##__VA_ARGS__); \
} while (0)

#define MUGGLE_LOG_TRACE(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_TRACE, format, ##__VA_ARGS__)
#define MUGGLE_LOG_DEBUG(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define MUGGLE_LOG_INFO(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define MUGGLE_LOG_WARNING(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define MUGGLE_LOG_ERROR(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define MUGGLE_LOG_FATAL(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_FATAL, format, ##__VA_ARGS__)


#if MUGGLE_RELEASE

#define MUGGLE_ASSERT(x)
#define MUGGLE_ASSERT_MSG(x, format, ...)
#define MUGGLE_DEBUG_LOG_TRACE(format, ...)
#define MUGGLE_DEBUG_LOG_DEBUG(format, ...)
#define MUGGLE_DEBUG_LOG_INFO(format, ...)
#define MUGGLE_DEBUG_LOG_WARNING(format, ...)
#define MUGGLE_DEBUG_LOG_ERROR(format, ...)
#define MUGGLE_DEBUG_LOG_FATAL(format, ...)
#define MUGGLE_DEBUG_LOG(ptr_log_handle, level, format, ...)

#else

#define MUGGLE_ASSERT(x) \
do \
{ \
	if (!(x)) \
	{ \
		muggle_log_src_loc_t loc_arg##__LINE__ = { \
			__FILE__, __LINE__, __FUNCTION__ \
		}; \
		muggle_logger_t *logger = muggle_logger_default(); \
		logger->log(logger, MUGGLE_LOG_LEVEL_FATAL, &loc_arg##__LINE__, "Assertion: "#x); \
	} \
} while (0)

#define MUGGLE_ASSERT_MSG(x, format, ...) \
do \
{ \
	if (!(x)) \
	{ \
		muggle_log_src_loc_t loc_arg##__LINE__ = { \
			__FILE__, __LINE__, __FUNCTION__ \
		}; \
		muggle_logger_t *logger = muggle_logger_default(); \
		logger->log(logger, MUGGLE_LOG_LEVEL_FATAL, &loc_arg##__LINE__, "Assertion: "#x ", " format, ##__VA_ARGS__); \
	} \
} while (0)

#define MUGGLE_DEBUG_LOG_TRACE(format, ...) MUGGLE_LOG_TRACE(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_LOG_DEBUG(format, ...) MUGGLE_LOG_DEBUG(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_LOG_INFO(format, ...) MUGGLE_LOG_INFO(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_LOG_WARNING(format, ...) MUGGLE_LOG_WARNING(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_LOG_ERROR(format, ...) MUGGLE_LOG_ERROR(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_LOG_FATAL(format, ...) MUGGLE_LOG_FATAL(format, ##__VA_ARGS__)
#define MUGGLE_DEBUG_LOG(logger, level, format, ...) MUGGLE_LOG(logger, level, format, ##__VA_ARGS__)

#endif

#if MUGGLE_HOLD_LOG_MACRO

// use mugglec log by default
#define LOG_TRACE(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_TRACE, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) MUGGLE_LOG_DEFAULT(MUGGLE_LOG_LEVEL_FATAL, format, ##__VA_ARGS__)

#define DEBUG_LOG_TRACE(format, ...) MUGGLE_DEBUG_LOG_TRACE(format, ##__VA_ARGS__)
#define DEBUG_LOG_DEBUG(format, ...) MUGGLE_DEBUG_LOG_DEBUG(format, ##__VA_ARGS__)
#define DEBUG_LOG_INFO(format, ...) MUGGLE_DEBUG_LOG_INFO(format, ##__VA_ARGS__)
#define DEBUG_LOG_WARNING(format, ...) MUGGLE_DEBUG_LOG_WARNING(format, ##__VA_ARGS__)
#define DEBUG_LOG_ERROR(format, ...) MUGGLE_DEBUG_LOG_ERROR(format, ##__VA_ARGS__)
#define DEBUG_LOG_FATAL(format, ...) MUGGLE_DEBUG_LOG_FATAL(format, ##__VA_ARGS__)

#define LOG_LEVEL_TRACE MUGGLE_LOG_LEVEL_TRACE
#define LOG_LEVEL_DEBUG MUGGLE_LOG_LEVEL_DEBUG
#define LOG_LEVEL_INFO MUGGLE_LOG_LEVEL_INFO
#define LOG_LEVEL_WARNING MUGGLE_LOG_LEVEL_WARNING
#define LOG_LEVEL_ERROR MUGGLE_LOG_LEVEL_ERROR
#define LOG_LEVEL_FATAL MUGGLE_LOG_LEVEL_FATAL

#endif

/**
 * @brief simple initialize log
 *
 * invoke this function, will initialize default sync logger
 * and add console handler and file rotate handler
 *
 * @param level_console       console output level, if it's -1, disable console output
 * @param level_file_rotating file rotating output level, if it's -1, disable file output
 *
 * @return 
 *     0 - success
 *     otherwise - failed
 */
MUGGLE_C_EXPORT
int muggle_log_simple_init(int level_console, int level_file_rotating);

/**
 * @brief complicated initialize log
 *
 * @param level_console       console output level, if it's -1, disable console output
 * @param level_file_rotating file time rotating output level, if it's -1, disable file output
 * @param log_path            log output path, if it's NULL, log path is $PWD/log/binary_name.log.yyyymmdd
 *
 * @return 
 *     0 - success
 *     otherwise - failed
 */
MUGGLE_C_EXPORT
int muggle_log_complicated_init(
	int level_console,
	int level_file_time_rotating,
	const char *log_path);

EXTERN_C_END

#endif
