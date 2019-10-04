/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_ERR_H_
#define MUGGLE_C_ERR_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_OK = 0,
	MUGGLE_ERR_UNKNOWN,
	MUGGLE_ERR_TODO,
	MUGGLE_ERR_MEM_ALLOC,
	MUGGLE_ERR_INVALID_PARAM,
	MUGGLE_ERR_ACQ_LOCK,
	MUGGLE_ERR_SYS_CALL,
	MUGGLE_ERR_INTERRUPT,
	MUGGLE_ERR_BEYOND_RANGE,
	MUGGLE_ERR_MAX,
};

EXTERN_C_END

#endif
