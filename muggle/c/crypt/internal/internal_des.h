/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_INTERNAL_DES_H_
#define MUGGLE_C_INTERNAL_DES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"
#include "muggle/c/crypt/des.h"

EXTERN_C_BEGIN

/*
 * move single bit
 * */
#define MOVE_SINGLE_BIT(in, from, to) ((((in)>>from)&0x01)<<to)

/*
 * key shift
 * */
#define DES_KEY_SHIFT(in, shift, mask) (((in)<<(shift))|(((in)>>(28-(shift)))&(mask)))


/*
 * DES Initial Permutation
 * @in: input block
 * @out: output block
 * */
MUGGLE_CC_EXPORT
void muggle_des_ip(const muggle_64bit_block_t *in, muggle_64bit_block_t *out);

/*
 * DES Final Permutation(Inverse IP)
 * @in: input block
 * @out: output block
 * */
MUGGLE_CC_EXPORT
void muggle_des_fp(const muggle_64bit_block_t *in, muggle_64bit_block_t *out);

/*
 * DES Expand Permutation
 * @in: input block
 * @out: 48bits in 8 bytes
 * */
MUGGLE_CC_EXPORT
void muggle_des_expand(const muggle_32bit_block_t *in, muggle_des_subkey_t *out);

/*
 * DES S-Box
 * @in: input 48bits
 * @out: 4bit x 8 output
 * */
MUGGLE_CC_EXPORT
void muggle_des_sbox(const muggle_des_subkey_t *in, muggle_des_subkey_t *out);

/*
 * DES P permutation
 * @in: output of S-Box
 * @out: output block
 * */
MUGGLE_CC_EXPORT
void muggle_des_p(const muggle_32bit_block_t *in, muggle_32bit_block_t *out);

/*
 * DES f function
 * @in: input block
 * @out: output block
 * */
MUGGLE_CC_EXPORT
void muggle_des_f(const muggle_32bit_block_t *in, const muggle_des_subkey_t *sk, muggle_32bit_block_t *out);

/*
 * DES PC-1
 * @in: input block
 * @out: output block
 * */
MUGGLE_CC_EXPORT
void muggle_des_pc1(const muggle_64bit_block_t *in, muggle_64bit_block_t *out);

/*
 * DES PC-2
 * @k: contain 28bit in k->u32.l and 28bit in k->u32.h
 * @sk: output sub key
 * */
MUGGLE_CC_EXPORT
void muggle_des_pc2(muggle_64bit_block_t *k, muggle_des_subkey_t *sk);

EXTERN_C_END

#endif
