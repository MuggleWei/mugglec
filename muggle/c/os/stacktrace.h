/******************************************************************************
 *  @file         stacktrace.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec stacktrace
 *****************************************************************************/

#ifndef MUGGLE_C_STACKTRACE_H_
#define MUGGLE_C_STACKTRACE_H_

#include "muggle/c/base/macro.h"

#define MUGGLE_MAX_STACKTRACE_FRAME_NUM 256

EXTERN_C_BEGIN

/**
 * @brief stack trace structure
 */
typedef struct muggle_stacktrace
{
	unsigned int cnt_frame;
	char         **symbols;
}muggle_stacktrace_t;

/**
 * @brief 
 * get stack trace information, after use, remember free stack trace
 * info with muggle_stacktrace_free
 *
 * @param st             stack trace pointer
 * @param max_cnt_frame  want to get max frame number, if frame == 0, use MUGGLE_MAX_STACKTRACE_FRAME_NUM
 *
 * @return on success, return the number of frames, on error, -1 is returned
 */
MUGGLE_C_EXPORT
int muggle_stacktrace_get(muggle_stacktrace_t *st, unsigned int max_cnt_frame);

/**
 * @brief free stack trace info
 *
 * @param st  stack trace pointer
 */
MUGGLE_C_EXPORT
void muggle_stacktrace_free(muggle_stacktrace_t *st);

/**
 * @brief output stack trace info to stdout
 */
MUGGLE_C_EXPORT
void muggle_print_stacktrace();

EXTERN_C_END

#endif
