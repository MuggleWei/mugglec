/******************************************************************************
 *  @file         sys.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-07-29
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sys
 *****************************************************************************/

#ifndef MUGGLE_C_SYS_H_
#define MUGGLE_C_SYS_H_

#include "muggle/c/base/macro.h"
#include <stddef.h>
#include "muggle/c/log/log.h"

EXTERN_C_BEGIN

/**
 * @brief system last error no
 *
 * @return err no
 */
MUGGLE_C_EXPORT
int muggle_sys_lasterror();

/**
 * @brief get string description for system error num
 *
 * @param errnum   system error number
 * @param buf      buffer that stores error string
 * @param bufsize  size of buffer
 *
 * @return returns 0 on success, otherwise failed
 */
MUGGLE_C_EXPORT
int muggle_sys_strerror(int errnum, char *buf, size_t bufsize);

/**
 * @brief output system last error message
 *
 * @param log_level  log level, see MUGGLE_LOG_LEVEL_*
 * @param msg        user message
 *
 * @note
 * for output line, function infos, use macro
 */
#define MUGGLE_LOG_SYS_ERR(log_level, msg) \
{ \
	int last_errnum = muggle_sys_lasterror(); \
	char err_msg_buf[512]; \
	if (muggle_sys_strerror(last_errnum, err_msg_buf, sizeof(err_msg_buf)) != -1) \
	{ \
		MUGGLE_LOG_DEFAULT(log_level, "%s: %s", msg, err_msg_buf); \
	} \
}

#if MUGGLE_RELEASE
#define MUGGLE_DEBUG_LOG_SYS_ERR(log_level, msg)
#else
#define MUGGLE_DEBUG_LOG_SYS_ERR(log_level, msg) MUGGLE_LOG_SYS_ERR(log_level, msg)
#endif

#if MUGGLE_HOLD_LOG_MACRO
#define LOG_SYS_ERR(log_level, msg) MUGGLE_LOG_SYS_ERR(log_level, msg)
#define DEBUG_LOG_SYS_ERR(log_level, msg) MUGGLE_DEBUG_LOG_SYS_ERR(log_level, msg)
#endif

EXTERN_C_END

#endif /* ifndef MUGGLE_C_SYS_H_ */
