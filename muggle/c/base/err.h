/******************************************************************************
 *  @file         err.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec err
 *****************************************************************************/

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
	MUGGLE_ERR_MEM_DUPLICATE_FREE,
	MUGGLE_ERR_NULL_PARAM,
	MUGGLE_ERR_INVALID_PARAM,
	MUGGLE_ERR_ACQ_LOCK,
	MUGGLE_ERR_SYS_CALL,
	MUGGLE_ERR_INTERRUPT,
	MUGGLE_ERR_BEYOND_RANGE,
	MUGGLE_ERR_FULL,

	MUGGLE_ERR_CRYPT_PLAINTEXT_SIZE, // invalid plaintext size
	MUGGLE_ERR_CRYPT_KEY_SIZE,       // invalid key size

	MUGGLE_ERR_MAX,
};

EXTERN_C_END

#endif
