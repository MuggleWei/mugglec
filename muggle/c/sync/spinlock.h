/******************************************************************************
 *  @file         spinlock.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-01-05
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec spinlock
 *****************************************************************************/

#ifndef MUGGLE_C_SPINLOCK_H_
#define MUGGLE_C_SPINLOCK_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_SPINLOCK_STATUS_UNLOCK,
	MUGGLE_SPINLOCK_STATUS_LOCK,
};

/**
 * @brief initialize spinlock status
 *
 * @param spinlock  spinlock
 */
MUGGLE_C_EXPORT
void muggle_spinlock_init(muggle_atomic_int *spinlock);

/**
 * @brief lock spinlock
 *
 * @param spinlock  spinlock
 */
MUGGLE_C_EXPORT
void muggle_spinlock_lock(muggle_atomic_int *spinlock);

/**
 * @brief unlock spinlock
 *
 * @param spinlock  spinlock
 */
MUGGLE_C_EXPORT
void muggle_spinlock_unlock(muggle_atomic_int *spinlock);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_SPINLOCK_H_ */
