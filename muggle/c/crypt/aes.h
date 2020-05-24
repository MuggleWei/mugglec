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

EXTERN_C_BEGIN

#define MUGGLE_AES_BLOCK_SIZE 16
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

/**
 * AES crypt with CBC mode
 * @param ctx AES context
 * @param input input bytes, length must be multiple of 16
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector
 * @param output output bytes
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_aes_cbc(
	const muggle_aes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_AES_BLOCK_SIZE],
	unsigned char *output);

/**
 * AES crypt with CFB128 mode
 * @param ctx AES context
 * @param input input bytes, length must be multiple of 16
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector
 * @param iv_offset offset bytes in iv
 * @param output output bytes
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_aes_cfb128(
	const muggle_aes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_AES_BLOCK_SIZE],
	unsigned int *iv_offset,
	unsigned char *output);

/**
 * AES crypt with OFB128 mode
 * @param ctx AES context
 * @param input input bytes, length must be multiple of 16
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector
 * @param iv_offset offset bytes in iv
 * @param output output bytes
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_aes_ofb128(
	const muggle_aes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_AES_BLOCK_SIZE],
	unsigned int *iv_offset,
	unsigned char *output);

/**
 * AES crypt with CTR mode
 * @param ctx AES context
 * @param input input bytes, length must be multiple of 16
 * @param num_bytes length of input/output bytes
 * @param nonce
 * @param nonce_offset offset bytes in nonce
 * @param output output bytes
 * @param stream_block ciphertext of nonce
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_aes_ctr(
	const muggle_aes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	uint64_t nonce[2],
	unsigned int *nonce_offset,
	unsigned char stream_block[MUGGLE_AES_BLOCK_SIZE],
	unsigned char *output);

EXTERN_C_END

#endif
