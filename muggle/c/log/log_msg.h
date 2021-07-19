/******************************************************************************
 *  @file         log_msg.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-07-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log msg
 *****************************************************************************/

#ifndef MUGGLE_C_LOG_MSG_H_
#define MUGGLE_C_LOG_MSG_H_

#include "muggle/c/base/macro.h"
#include <time.h>
#include "muggle/c/base/thread.h"

EXTERN_C_BEGIN

#define MUGGLE_LOG_MSG_MAX_LEN 4096

/**
 * @brief source location info
 */
typedef struct muggle_log_src_loc
{
	const char   *file;  //!< source file
	unsigned int line;   //!< source file line
	const char   *func;  //!< source function name
}muggle_log_src_loc_t;

/**
 * @brief log message
 */
typedef struct muggle_log_msg
{
	int                  level;        //!< log level
	struct timespec      ts;           //!< timestamp
	muggle_thread_id     tid;          //!< thread id
	muggle_log_src_loc_t src_loc;      //!< source location info
	const char           *payload;     //!< payload
}muggle_log_msg_t;

EXTERN_C_END

#endif /* ifndef MUGGLE_C_LOG_MSG_H_ */
