/******************************************************************************
 *  @file         log_category.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log category
 *****************************************************************************/
 
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
	int lowest_log_level;
}muggle_log_category_t;

/**
 * @brief add log handle into log category
 *
 * @param category log category
 * @param handle   log handle
 *
 * @return success returns 0, otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_category_add(muggle_log_category_t *category, muggle_log_handle_t *handle);

/**
 * @brief destroy log category
 *
 * @param category        log category
 * @param delete_handles  nonzero represent need delete all handles in this category
 *
 * @return success returns 0, otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_category_destroy(muggle_log_category_t *category, int delete_handles);

/**
 * @brief output message
 *
 * @param category log category
 * @param arg      log format arguments
 * @param msg      log messages
 *
 * @return  success returns 0, otherwise return err code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_log_category_write(
	muggle_log_category_t *category,
	muggle_log_fmt_arg_t *arg,
	const char *msg
);

EXTERN_C_END

#endif
