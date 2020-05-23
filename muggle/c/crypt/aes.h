/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_CRYPT_AES_H_
#define MUGGLE_C_CRYPT_AES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"

#define MUGGLE_AES_BLOCK_SIZE 16

EXTERN_C_BEGIN

#define MUGGLE_AES_MAX_NUM_ROUND 14

typedef struct muggle_aes_sub_keys
{
	uint32_t rd_key[4 * (MUGGLE_AES_MAX_NUM_ROUND + 1)];
	int rounds;
}muggle_aes_subkeys_t;

typedef struct muggle_aes_context
{
	int                  op;   // encryption or decryption, use MUGGLE_DECRYPT or MUGGLE_ENCRYPT
	int                  mode; // cipher block mode, use MUGGLE_BLOCK_CIPHER_MODE_*
	muggle_aes_subkeys_t sk;   // AES subkeys
}muggle_aes_context_t;

/*
 * AES setup round keys for mode
 * @param op
 * - MUGGLE_ENCRYPT encrypt
 * - MUGGLE_DECRYPT decrypt
 * @param mode block cipher mode, see MUGGLE_BLOCK_CIPHER_MODE_*
 * @param key user input key
 * @param bits number bits of key (128|192|256)
 * @param ctx AES context
 * @return
 *   - 0 success
 *   - otherwise failed, see MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_aes_set_key(
	int op,
	int mode,
	const unsigned char *key,
	int bits,
	muggle_aes_context_t *ctx);

/**
 * AES crypt with ECB mode
 * @param ctx AES context
 * @param input input bytes, length must be multiple of 16
 * @param num_bytes length of input/output bytes
 * @param output output bytes
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_aes_ecb(
	const muggle_aes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char *output);

EXTERN_C_END

#endif
