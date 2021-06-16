/******************************************************************************
 *  @file         tdes.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec crypt TDES
 *****************************************************************************/
 
#ifndef MUGGLE_C_TDES_H_
#define MUGGLE_C_TDES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"
#include "muggle/c/crypt/des.h"

EXTERN_C_BEGIN

typedef struct muggle_tdes_context
{
	int                  op;   // encryption or decryption, use MUGGLE_DECRYPT or MUGGLE_ENCRYPT
	int                  mode; // cipher block mode, use MUGGLE_BLOCK_CIPHER_MODE_*
	muggle_des_context_t ctx1; // DES subkeys
	muggle_des_context_t ctx2; // DES subkeys
	muggle_des_context_t ctx3; // DES subkeys
}muggle_tdes_context_t;

/**
 * @brief TDES setup key schedule for mode
 *
 * @param op crypt operator
 *   - MUGGLE_DECRYPT encrypt
 *   - MUGGLE_ENCRYPT decrypt
 * @param mode block cipher mode, see MUGGLE_BLOCK_CIPHER_MODE_*
 * @param key des input key
 * @param ctx DES context
 *
 * @return
 *   - 0 success
 *   - otherwise failed, see MUGGLE_ERR_*
 */
MUGGLE_C_EXPORT
int muggle_tdes_set_key(
	int op,
	int mode,
	const unsigned char key1[MUGGLE_DES_BLOCK_SIZE],
	const unsigned char key2[MUGGLE_DES_BLOCK_SIZE],
	const unsigned char key3[MUGGLE_DES_BLOCK_SIZE],
	muggle_tdes_context_t *ctx);

/**
 * @brief TDES crypt with ECB mode
 *
 * @param ctx TDES context
 * @param input input bytes, length must be multiple of 8
 * @param num_bytes length of input/output bytes
 * @param output output bytes
 *
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 */
MUGGLE_C_EXPORT
int muggle_tdes_ecb(
	muggle_tdes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char *output);

/**
 * @brief TDES crypt with CBC mode
 *
 * @param ctx TDES context
 * @param input input bytes, length must be multiple of 8
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector
 * @param iv_offset offset bytes in iv
 * @param output output bytes
 *
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 */
MUGGLE_C_EXPORT
int muggle_tdes_cbc(
	const muggle_tdes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE],
	unsigned char *output);

/**
 * @brief TDES crypt with CFB 64bit iv mode
 *
 * @param ctx TDES context
 * @param input input bytes
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector
 * @param iv_offset offset bytes in iv
 * @param output output bytes
 *
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 * */
MUGGLE_C_EXPORT
int muggle_tdes_cfb64(
	const muggle_tdes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE],
	unsigned int *iv_offset,
	unsigned char *output);

/**
 * @brief TDES crypt with OFB 64bit iv mode
 *
 * @param ctx TDES context
 * @param input input bytes
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector
 * @param iv_offset offset bytes in iv
 * @param output output bytes
 *
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 */
MUGGLE_C_EXPORT
int muggle_tdes_ofb64(
	const muggle_tdes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE],
	unsigned int *iv_offset,
	unsigned char *output);

/**
 * @brief TDES crypt with CTR mode
 *
 * @param ctx TDES context
 * @param input input bytes
 * @param num_bytes length of input/output bytes
 * @param nonce
 * @param nonce_offset offset bytes in nonce
 * @param output output bytes
 * @param stream_block ciphertext of nonce
 *
 * @return
 *   - 0 success
 *   - otherwise failed, return MUGGLE_ERR_*
 */
MUGGLE_C_EXPORT
int muggle_tdes_ctr(
	const muggle_tdes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	uint64_t *nonce,
	unsigned int *nonce_offset,
	unsigned char stream_block[MUGGLE_DES_BLOCK_SIZE],
	unsigned char *output);

EXTERN_C_END

#endif
