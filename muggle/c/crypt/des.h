/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_DES_H_
#define MUGGLE_C_DES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"

EXTERN_C_BEGIN

typedef struct muggle_des_subkey
{
	unsigned char bytes[8];
}muggle_des_subkey_t;

typedef struct muggle_des_subkeys
{
	muggle_des_subkey_t sk[16];
}muggle_des_subkeys_t;

/*
 * DES key schedule
 * */
MUGGLE_CC_EXPORT
void muggle_des_gen_subkeys(const muggle_64bit_block_t *key, muggle_des_subkeys_t *subkeys);


/*
 * DES encrypt/decrypt a single 64bit block
 * @enc: MUGGLE_ENCRYPT - encrypt input, MUGGLE_DECRYPT - decrypt input
 * @input: input block
 * @ks: sub key schedule
 * @output: output block
 * RETURN: 0 represents success, otherwise failed
 * */
MUGGLE_CC_EXPORT
int muggle_des_crypt(
	int enc,
	const muggle_64bit_block_t *input,
	const muggle_des_subkeys_t *ks,
	muggle_64bit_block_t *output);

// /*
//  * DES encrypt/decrypt in mode
//  * @mode: block cipher mode, use MUGGLE_BLOCK_CIPHER_MODE_*
//  * @enc: MUGGLE_ENCRYPT - encrypt input, MUGGLE_DECRYPT - decrypt input
//  * @input: plaintext bytes
//  * @output: ciphertext bytes
//  * @num_bytes: length of input/output bytes
//  * @key: key
//  * @iv: initialization vector;
//  * 	ECB: doesn't make sense in ECB mode
//  * 	CTR: equal to init counter values
//  * @update_iv: 1 - update, 0 - don't update;
//  *  ECB: doesn't make sense in ECB mode
//  *  CTR: whether CTR need update counter values
//  * @callbacks: if NULL, use default callbacks
//  * RETURN: 
//  *  return 0 on success, otherwise failed
//  *
//  * NOTE: Don't use ECB mode in product environment
//  * */
// MUGGLE_CC_EXPORT
// int muggle_des_cipher(
// 	int mode,
// 	int enc,
// 	const unsigned char *input,
// 	unsigned char *output,
// 	unsigned int num_bytes,
// 	muggle_des_key_block key,
// 	muggle_64bit_block_t *iv,
// 	int update_iv,
// 	muggle_des_cb_t *callbacks);

EXTERN_C_END

#endif
