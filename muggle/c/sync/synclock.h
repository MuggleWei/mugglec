/******************************************************************************
 *  @file         synclock.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-11-24
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sync lock
 *
 *  NOTE: Don't use it immediatelly, cause protability not guaranteed
 *****************************************************************************/

#ifndef MUGGLE_C_SYNCLOCK_H_
#define MUGGLE_C_SYNCLOCK_H_


#include "muggle/c/base/macro.h"
#include "muggle/c/sync/sync_obj.h"

EXTERN_C_BEGIN

#if MUGGLE_C_HAVE_SYNC_OBJ

/**
 * @brief initialize synclock status
 *
 * @param synclock  synclock
 */
MUGGLE_C_EXPORT
void muggle_synclock_init(muggle_sync_t *synclock);

/**
 * @brief lock synclock
 *
 * @param synclock  synclock
 */
MUGGLE_C_EXPORT
void muggle_synclock_lock(muggle_sync_t *synclock);

/**
 * @brief unlock synclock
 *
 * @param synclock  synclock
 */
MUGGLE_C_EXPORT
void muggle_synclock_unlock(muggle_sync_t *synclock);

#endif // MUGGLE_C_HAVE_SYNC_OBJ

EXTERN_C_END

#endif // !MUGGLE_C_SYNCLOCK_H_
