/******************************************************************************
 *  @file         log_utils.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log utils
 *****************************************************************************/
 
#ifndef MUGGLE_C_LOG_UTILS_H_
#define MUGGLE_C_LOG_UTILS_H_

#include "muggle/c/log/log.h"

EXTERN_C_BEGIN

/**
 * @brief simple initialize log with console and file_rotate category
 *
 * @param level_console       console output level
 * @param level_file_rotating file rotating output level
 *
 * @return 
 *     0 - success
 *     otherwise - failed
 */
MUGGLE_C_EXPORT
int muggle_log_simple_init(int level_console, int level_file_rotating);

EXTERN_C_END

#endif
