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

typedef muggle_64bit_block_t muggle_des_key_block;

typedef struct muggle_des_subkeys
{
	muggle_64bit_block_t subkey[16];
}muggle_des_subkeys_t;

/*
 * DES init/final permutation callback function
 * RETURN: 0 - success, otherwise - failed
 * */
typedef int (*fn_muggle_des_ifp)(muggle_64bit_block_t *block);

/*
 * DES key generate sub key callback function
 * RETURN: 0 - success, otherwise - failed
 * */
typedef int (*fn_muggle_des_gen_subkey)(const muggle_64bit_block_t *key, muggle_des_subkeys_t *ks);

/*
 * DES extend permutation
 * RETURN: extend result, save in 64bit block
 * */
typedef muggle_64bit_block_t (*fn_muggle_des_ep)(uint32_t r);

/*
 * DES SBox
 * */
typedef uint32_t (*fn_muggle_des_sbox)(muggle_64bit_block_t block);

typedef struct muggle_des_callbacks
{
	fn_muggle_des_ifp        ip;         // DES initalize permutation
	fn_muggle_des_ifp        fp;         // DES final permutation
	fn_muggle_des_gen_subkey gen_subkey; // DES generate subkey schedule from key
	fn_muggle_des_ep         ep;         // DES extend permutation
	fn_muggle_des_sbox       sbox;       // DES S-Box
}muggle_des_cb_t;


/*
 * DES encrypt/decrypt a single 64bit block
 * @enc: MUGGLE_ENCRYPT - encrypt input, MUGGLE_DECRYPT - decrypt input
 * @input: input block
 * @output: output block
 * @ks: sub key schedule
 * @callbacks: DES callback functions, if NULL, use default callbacks(compatible with openssl)
 * */
int muggle_des_crypt(
	int enc,
	const muggle_64bit_block_t *input,
	muggle_64bit_block_t *output,
	muggle_des_subkeys_t *ks,
	muggle_des_cb_t *callbacks);

/*
 * DES encrypt/decrypt in ECB mode
 * @enc: MUGGLE_ENCRYPT - encrypt input, MUGGLE_DECRYPT - decrypt input
 * @input: plaintext bytes
 * @output: ciphertext bytes
 * @num_bytes: length of input/output bytes
 * @key: key
 * @callbacks: if NULL, use default callbacks
 * RETURN: 
 *  return 0 on success, otherwise failed
 *
 * NOTE: Don't use ECB mode in product environment
 * */
int muggle_des_ecb(
	int enc,
	const unsigned char *input,
	unsigned char *output,
	unsigned int num_bytes,
	muggle_des_key_block key,
	muggle_des_cb_t *callbacks);

/*
 * DES encrypt/decrypt in CBC mode
 * @enc: 1 - encrypt, 0 - decrypt
 * @input: plaintext block
 * @output: ciphertext block
 * @num_bytes: length of input/output bytes
 * @key: key
 * @iv: initialization vector
 * @update_iv: 1 - update, 0 - don't update
 * @callbacks: if NULL, use default callbacks
 * RETURN: 
 *  return 0 on success, otherwise failed
 *
 * NOTE: Don't use ECB mode in product environment
 * */
int muggle_des_cbc(
	int enc,
	const unsigned char *input,
	unsigned char *output,
	unsigned int num_bytes,
	muggle_des_key_block key,
	muggle_64bit_block_t *iv,
	int update_iv,
	muggle_des_cb_t *callbacks);

EXTERN_C_END

#endif
