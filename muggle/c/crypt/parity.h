/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_PARITY_H_
#define MUGGLE_C_PARITY_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/*
 * output odd/event parity array, for test, don't use
 * it in real project
 * */
MUGGLE_CC_EXPORT
void muggle_output_odd_parity();

MUGGLE_CC_EXPORT
void muggle_output_even_parity();

/*
 * parity check
 * RETURN: 1 - check success, 0 - check failed
 * */
MUGGLE_CC_EXPORT
int muggle_parity_check_odd(unsigned char b);

MUGGLE_CC_EXPORT
int muggle_parity_check_even(unsigned char b);

/*
 * set parity
 * */
MUGGLE_CC_EXPORT
unsigned char muggle_parity_set_odd(unsigned char b);

MUGGLE_CC_EXPORT
unsigned char muggle_parity_set_even(unsigned char b);

EXTERN_C_END

#endif
