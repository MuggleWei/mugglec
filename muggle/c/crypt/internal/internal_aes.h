/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_INTERNAL_AES_H_
#define MUGGLE_C_INTERNAL_AES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"

EXTERN_C_BEGIN

/*
 * AES SubBytes Transformation
 * @param state a rectangular array of 16 bytes
 * */
void muggle_aes_sub_bytes(unsigned char *state);

/*
 * AES InvSubBytes Transformation
 * @param state a rectangular array of 16 bytes
 * */
void muggle_aes_inv_sub_bytes(unsigned char *state);

/*
 * AES Shift Rows
 * @param state a rectangular array of 16 bytes
 * */
void muggle_aes_shift_rows(unsigned char *state);

/*
 * AES Inv Shift Rows
 * @param state a rectangular array of 16 bytes
 * */
void muggle_aes_inv_shift_rows(unsigned char *state);

/*
 * AES Mix Column
 * @param state a rectangular array of 16 bytes
 * */
void muggle_aes_mix_column(unsigned char *state);

/*
 * AES Inv Mix Column
 * @param state a rectangular array of 16 bytes
 * */
void muggle_aes_inv_mix_column(unsigned char *state);

EXTERN_C_END

#endif
