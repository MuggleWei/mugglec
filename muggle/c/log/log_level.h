/******************************************************************************
 *  @file         log_level.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log level
 *****************************************************************************/

#ifndef MUGGLE_C_LOG_LEVEL_H_
#define MUGGLE_C_LOG_LEVEL_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_LOG_LEVEL_OFFSET = 8,
	MUGGLE_LOG_LEVEL_TRACE = 0,
	MUGGLE_LOG_LEVEL_DEBUG = 1 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_INFO = 2 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_WARNING = 3 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_WARN = MUGGLE_LOG_LEVEL_WARNING,
	MUGGLE_LOG_LEVEL_ERROR = 4 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_FATAL = 5 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_MAX = 6,
};

/**
 * @brief get log level string representation
 *
 * @param log_level  log level
 *
 * @return 
 */
MUGGLE_C_EXPORT
const char* muggle_log_level_to_str(int log_level);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_LOG_LEVEL_H_ */
