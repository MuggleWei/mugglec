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

#define MUGGLE_CRYPT_AES_DEBUG 0

struct muggle_aes_sub_keys;


/*
 * AES Add Round Key
 * */
void muggle_aes_add_round_key(unsigned char *state, const uint32_t *rd_key);

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

/*
 * AES RotWord
 * */
uint32_t muggle_aes_rot_word(uint32_t word);

/*
 * AES SubWord
 * */
uint32_t muggle_aes_sub_word(uint32_t word);

/*
 * AES encrypt
 * */
int muggle_aes_encrypt(
	unsigned char *state,
	const struct muggle_aes_sub_keys *sk);

/*
 * AES decrypt
 * */
int muggle_aes_decrypt(
	unsigned char *state,
	const struct muggle_aes_sub_keys *sk);


EXTERN_C_END

#endif
