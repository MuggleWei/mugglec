/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_STACKTRACE_H_
#define MUGGLE_C_STACKTRACE_H_

#include "muggle/c/base/macro.h"

#define MUGGLE_MAX_STACKTRACE_FRAME_NUM 256

EXTERN_C_BEGIN

typedef struct muggle_stacktrace
{
	unsigned int cnt_frame;
	char         **symbols;
}muggle_stacktrace_t;

/*
 * get stack trace information, after use, remember free
 * stack trace info with muggle_stacktrace_free
 * @max_cnt_frame: want to get max frame number, if frame == 0, use MUGGLE_MAX_STACKTRACE_FRAME_NUM
 * RETURN: on success, return the number of frames, on error, -1 is returned
 * */
MUGGLE_CC_EXPORT
int muggle_stacktrace_get(muggle_stacktrace_t *st, unsigned int max_cnt_frame);

/*
 * free stack trace info
 * */
MUGGLE_CC_EXPORT
void muggle_stacktrace_free(muggle_stacktrace_t *st);

/*
 * output stack trace info to stdout
 * */
MUGGLE_CC_EXPORT
void muggle_print_stacktrace();

EXTERN_C_END

#endif
