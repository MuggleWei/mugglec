/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_LOG_FMT_H_
#define MUGGLE_C_LOG_FMT_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/thread.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_LOG_LEVEL_OFFSET = 8,
	MUGGLE_LOG_LEVEL_TRACE = 1 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_INFO = 2 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_WARNING = 3 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_ERROR = 4 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_FATAL = 5 << MUGGLE_LOG_LEVEL_OFFSET,
	MUGGLE_LOG_LEVEL_MAX = 6,
};

enum
{
	MUGGLE_LOG_FMT_LEVEL = 0x01,
	MUGGLE_LOG_FMT_FILE = 0x02,
	MUGGLE_LOG_FMT_FUNC = 0x04,
	MUGGLE_LOG_FMT_TIME = 0x08,
	MUGGLE_LOG_FMT_THREAD = 0x10,
};

typedef struct muggle_log_fmt_arg_tag
{
	int level;
	unsigned int line;
	const char *file;
	const char *func;
	muggle_thread_id tid;
}muggle_log_fmt_arg_t;

/*
 * generate formated message
 * @fmt_flag: format flag
 * @arg: format arguments
 * @msg: original message
 * @buf: the formated message output buffer
 * @size: the size of buf
 * RETURN: the len of formated message, negative represent failed
 * */
MUGGLE_CC_EXPORT
int muggle_log_fmt_gen(
	int fmt_flag, muggle_log_fmt_arg_t *arg,
	const char *msg, char *buf, int size);

EXTERN_C_END

#endif
