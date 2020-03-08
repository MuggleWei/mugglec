/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_PARITY_H_
#define MUGGLE_C_PARITY_H_

#include "muggle/c/muggle_c.h"

EXTERN_C_BEGIN

/*
 * output odd/event parity array, for test, don't use
 * it in real project
 * */
void muggle_output_odd_parity();
void muggle_output_even_parity();

/*
 * parity check
 * */
bool muggle_parity_check_odd(unsigned char b);
bool muggle_parity_check_even(unsigned char b);

/*
 * set parity
 * */
unsigned char muggle_parity_set_odd(unsigned char b);
unsigned char muggle_parity_set_even(unsigned char b);

EXTERN_C_END

#endif
