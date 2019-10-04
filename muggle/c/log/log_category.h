/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_LOG_CATEGORY_H_
#define MUGGLE_C_LOG_CATEGORY_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/log/log_fmt.h"
#include "muggle/c/log/log_handle.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_LOG_CATEGORY_MAX_HANDLE = 8,
};

typedef struct muggle_log_category_tag
{
	muggle_log_handle_t *handles[MUGGLE_LOG_CATEGORY_MAX_HANDLE];
	int cnt;
}muggle_log_category_t;

/*
 * add log handle into log category
 * @category: log category
 * @handle: log handle
 * RETURN: success returns 0, otherwise return err code in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_log_category_add(muggle_log_category_t *category, muggle_log_handle_t *handle);

/*
 * destroy log category
 * @category: log category
 * @delete_handles: nonzero represent need delete all handles in this category
 * RETURN: success returns 0, otherwise return err code in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_log_category_destroy(muggle_log_category_t *category, int delete_handles);

/*
 * output message
 * @category: log category
 * @arg: log format arguments
 * @msg: log messages
 * RETURN: success returns 0, otherwise return err code in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_log_category_write(
	muggle_log_category_t *category,
	muggle_log_fmt_arg_t *arg,
	const char *msg
);

EXTERN_C_END

#endif
