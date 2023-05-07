/******************************************************************************
 *  @file         call_once.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sync call_once
 *****************************************************************************/

#ifndef MUGGLE_C_SYNC_CALL_ONCE_H_
#define MUGGLE_C_SYNC_CALL_ONCE_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"

EXTERN_C_BEGIN

enum {
	MUGGLE_ONCE_FLAG_INIT = 0,
	MUGGLE_ONCE_FLAG_WAIT,
	MUGGLE_ONCE_FLAG_READY,
};

typedef muggle_atomic_byte muggle_once_flag;
typedef void (*muggle_once_func)();

MUGGLE_C_EXPORT
void muggle_call_once(muggle_once_flag *flag, muggle_once_func func);

EXTERN_C_END

#endif // !MUGGLE_C_SYNC_CALLONCE_H_
