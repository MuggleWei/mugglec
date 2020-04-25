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

typedef struct muggle_des_48bit
{
	unsigned char bytes[8];
}muggle_des_48bit_t;
typedef struct muggle_des_48bit muggle_des_subkey_t;

typedef struct muggle_des_subkeys
{
	muggle_des_subkey_t sk[16];
}muggle_des_subkeys_t;

/**
 * DES key schedule
 * @param mode encryption or decryption, use MUGGLE_DECRYPT or MUGGLE_ENCRYPT
 * @param key des input key
 * @param subkeys output subkeys
 * */
MUGGLE_CC_EXPORT
void muggle_des_gen_subkeys(
	int mode,
	const muggle_64bit_block_t *key,
	muggle_des_subkeys_t *subkeys);


/*
 * DES encrypt/decrypt a single 64bit block
 * @param input input block
 * @param ks sub key schedule
 * @param output output block
 * @return 0 represents success, otherwise failed
 * */
MUGGLE_CC_EXPORT
int muggle_des_crypt(
	const muggle_64bit_block_t *input,
	const muggle_des_subkeys_t *ks,
	muggle_64bit_block_t *output);

/*
 * DES encrypt/decrypt in mode
 * @param block_cipher_mode block cipher mode, use MUGGLE_BLOCK_CIPHER_MODE_*
 * @param op encryption or decryption, use MUGGLE_DECRYPT or MUGGLE_ENCRYPT
 * @param key des input key
 * @param input input bytes, length must be multiple of 8
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector;
 * 	ECB: doesn't make sense in ECB mode
 * 	CTR: equal to init counter values
 * @param update_iv 1 - update, 0 - don't update;
 *  ECB: doesn't make sense in ECB mode
 *  CTR: whether CTR need update counter values
 * @param output output bytes
 * RETURN: 
 *  return 0 on success, otherwise failed
 *
 * NOTE: Don't use ECB mode in product environment
 * */
MUGGLE_CC_EXPORT
int muggle_des_cipher(
	int block_cipher_mode,
	int op,
	muggle_64bit_block_t key,
	const unsigned char *input,
	unsigned int num_bytes,
	muggle_64bit_block_t *iv,
	int update_iv,
	unsigned char *output);

EXTERN_C_END

#endif
