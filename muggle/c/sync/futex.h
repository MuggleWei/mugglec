/******************************************************************************
 *  @file         futex.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec futex
 *****************************************************************************/

#ifndef MUGGLE_C_FUTEX_H_
#define MUGGLE_C_FUTEX_H_

#include "muggle/c/base/macro.h"
#include <time.h>
#include "muggle/c/base/atomic.h"

EXTERN_C_BEGIN

/**
 * @brief futex wait
 *
 * @param futex_addr  futex address
 * @param val         futex wait value
 * @param timeout     timeout
 *
 * @return
 */
MUGGLE_C_EXPORT
int muggle_futex_wait(muggle_atomic_int *futex_addr, muggle_atomic_int val, const struct timespec *timeout);

/**
 * @brief futex wake one thread
 *
 * @param futex_addr  futex address
 */
MUGGLE_C_EXPORT
void muggle_futex_wake_one(muggle_atomic_int *futex_addr);

/**
 * @brief futex wake all thread
 *
 * @param futex_addr  futex address
 */
MUGGLE_C_EXPORT
void muggle_futex_wake_all(muggle_atomic_int *futex_addr);

EXTERN_C_END

#endif
