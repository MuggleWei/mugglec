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

typedef muggle_atomic_byte muggle_spinlock_t;

/**
 * @brief initialize spinlock status
 *
 * @param spinlock  spinlock
 */
MUGGLE_C_EXPORT
void muggle_spinlock_init(muggle_spinlock_t *spinlock);

/**
 * @brief lock spinlock
 *
 * @param spinlock  spinlock
 */
MUGGLE_C_EXPORT
void muggle_spinlock_lock(muggle_spinlock_t *spinlock);

/**
 * @brief unlock spinlock
 *
 * @param spinlock  spinlock
 */
MUGGLE_C_EXPORT
void muggle_spinlock_unlock(muggle_spinlock_t *spinlock);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_SPINLOCK_H_ */
