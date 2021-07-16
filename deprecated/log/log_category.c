/******************************************************************************
 *  @file         log_category.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec log category
 *****************************************************************************/
 
#include "log_category.h"
#include "muggle/c/base/err.h"

int muggle_log_category_add(muggle_log_category_t *category, muggle_log_handle_t *handle)
{
	if (category->cnt >= (sizeof(category->handles) / sizeof(category->handles[0])))
	{
		return MUGGLE_ERR_BEYOND_RANGE;
	}

	category->handles[category->cnt++] = handle;
	if (handle->level < category->lowest_log_level)
	{
		category->lowest_log_level = handle->level;
	}
	return MUGGLE_OK;
}

int muggle_log_category_destroy(muggle_log_category_t *category, int delete_handles)
{
	int ret = 0;
	if (delete_handles)
	{
		for (int i = 0; i < category->cnt; ++i)
		{
			ret = muggle_log_handle_destroy(category->handles[i]);
			if (ret != MUGGLE_OK)
			{
				return ret;
			}
		}
	}
	category->cnt = 0;

	return MUGGLE_OK;
}

int muggle_log_category_write(
	muggle_log_category_t *category,
	muggle_log_fmt_arg_t *arg,
	const char *msg
)
{
	int ret = 0;
	for (int i = 0; i < category->cnt; ++i)
	{
		ret = muggle_log_handle_write(category->handles[i], arg, msg);
		if (ret != MUGGLE_OK)
		{
			return ret;
		}
	}

	return ret;
}
