/******************************************************************************
 *  @file         parity.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec parity check
 *****************************************************************************/
 
#ifndef MUGGLE_C_PARITY_H_
#define MUGGLE_C_PARITY_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/**
 * @brief
 *
 * output odd parity array, for test, don't use
 * it in real project
 */
MUGGLE_C_EXPORT
void muggle_output_odd_parity();

/**
 * @brief
 *
 * output event parity array, for test, don't use
 * it in real project
 */
MUGGLE_C_EXPORT
void muggle_output_even_parity();

/**
 * @brief odd parity check
 *
 * @return
 *     1 - check success
 *     0 - check failed
 */
MUGGLE_C_EXPORT
int muggle_parity_check_odd(unsigned char b);

/**
 * @brief even parity check
 *
 * @return
 *     1 - check success
 *     0 - check failed
 */
MUGGLE_C_EXPORT
int muggle_parity_check_even(unsigned char b);

/**
 * @brief set odd parity
 */
MUGGLE_C_EXPORT
unsigned char muggle_parity_set_odd(unsigned char b);

/**
 * @brief set even parity
 */
MUGGLE_C_EXPORT
unsigned char muggle_parity_set_even(unsigned char b);

EXTERN_C_END

#endif
