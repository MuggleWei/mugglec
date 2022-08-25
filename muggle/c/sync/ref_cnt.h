/******************************************************************************
 *  @file         ref_cnt.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-24
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec reference count
 *****************************************************************************/

#ifndef MUGGLE_C_REF_CNT_H_
#define MUGGLE_C_REF_CNT_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"

EXTERN_C_BEGIN

typedef muggle_atomic_int muggle_ref_cnt_t;

/**
 * @brief init reference count
 *
 * @param ref       reference count
 * @param init_val  initialize value
 *
 * @return
 *     - return 0 on success
 *     - otherwise failed
 */
MUGGLE_C_EXPORT
int muggle_ref_cnt_init(muggle_ref_cnt_t *ref, int init_val);

/**
 * @brief
 * increases the reference count by 1
 *
 * @param ref  reference count
 *
 * @return
 *     on success, return reference count after this call
 *     on failed, return -1.
 *     when reference count already 0, then try to retain will return -1
 */
MUGGLE_C_EXPORT
int muggle_ref_cnt_retain(muggle_ref_cnt_t *ref);

/**
 * @brief decreases the reference count by 1
 *
 * @param ref  reference count
 *
 * @return 
 *     on success, return reference count after this call
 *     on failed, return -1.
 *     when reference count already 0, then try to release will return -1
 */
MUGGLE_C_EXPORT
int muggle_ref_cnt_release(muggle_ref_cnt_t *ref);

/**
 * @brief load reference count value
 *
 * @param ref  reference count
 *
 * @return value of reference count
 */
MUGGLE_C_EXPORT
int muggle_ref_cnt_val(muggle_ref_cnt_t *ref);

/**
 * @brief load reference count value
 *
 * @param ref       reference count
 * @param memmodel  memory order; see: muggle_memory_order_*
 *
 * @return value of reference count
 */
MUGGLE_C_EXPORT
int muggle_ref_cnt_load(muggle_ref_cnt_t *ref, int memmodel);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_REF_CNT_H_ */
