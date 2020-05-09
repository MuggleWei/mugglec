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

#define MUGGLE_DES_BLOCK_SIZE 8

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

typedef struct muggle_des_context
{
	int                  op; // encryption or decryption, use MUGGLE_DECRYPT or MUGGLE_ENCRYPT
	muggle_des_subkeys_t sk; // DES subkeys
}muggle_des_context_t;

/**
 * DES setup key schedule
 * @param op crypt operator
 *   - MUGGLE_DECRYPT encrypt
 *   - MUGGLE_ENCRYPT decrypt
 * @param key des input key
 * @param ctx DES context
 * @return
 *   - 0 success
 *   - otherwise failed, see MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_des_set_key(
	int op,
	const unsigned char key[MUGGLE_DES_BLOCK_SIZE],
	muggle_des_context_t *ctx);

/**
 * DES setup key schedule for mode
 * @param op crypt operator
 *   - MUGGLE_DECRYPT encrypt
 *   - MUGGLE_ENCRYPT decrypt
 * @param mode block cipher mode, see MUGGLE_BLOCK_CIPHER_MODE_*
 * @param key des input key
 * @param ctx DES context
 * @return
 *   - 0 success
 *   - otherwise failed, see MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_des_set_key_with_mode(
	int op,
	int mode,
	const unsigned char key[MUGGLE_DES_BLOCK_SIZE],
	muggle_des_context_t *ctx);

/**
 * DES crypt with ECB mode
 * @param ctx DES context
 * @param input input bytes, length must be multiple of 8
 * @param num_bytes length of input/output bytes
 * @param output output bytes
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_des_ecb(
	const muggle_des_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char *output);

/**
 * DES crypt with CBC mode
 * @param ctx DES context
 * @param input input bytes, length must be multiple of 8
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector
 * @param iv_offset offset bytes in iv
 * @param output output bytes
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_des_cbc(
	const muggle_des_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE],
	unsigned char *output);

/**
 * DES crypt with CFB mode
 * @param ctx DES context
 * @param input input bytes, length must be multiple of 8
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector
 * @param iv_offset offset bytes in iv
 * @param output output bytes
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_des_cfb(
	const muggle_des_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE],
	unsigned int *iv_offset,
	unsigned char *output);

/**
 * DES crypt with OFB mode
 * @param ctx DES context
 * @param input input bytes, length must be multiple of 8
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector
 * @param iv_offset offset bytes in iv
 * @param output output bytes
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 * */
MUGGLE_CC_EXPORT
int muggle_des_ofb(
	const muggle_des_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE],
	unsigned int *iv_offset,
	unsigned char *output);

/**
 * DES crypt with CTR mode
 * @param ctx DES context
 * @param input input bytes, length must be multiple of 8
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
int muggle_des_ctr(
	const muggle_des_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	uint64_t *nonce,
	unsigned int *nonce_offset,
	unsigned char stream_block[MUGGLE_DES_BLOCK_SIZE],
	unsigned char *output);

EXTERN_C_END

#endif
