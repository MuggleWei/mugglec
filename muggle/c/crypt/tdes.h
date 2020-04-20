/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_TDES_H_
#define MUGGLE_C_TDES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"
#include "muggle/c/crypt/des.h"

EXTERN_C_BEGIN

// /*
//  * Triple DES encrypt/decrypt a single 64bit block
//  * @enc: MUGGLE_ENCRYPT - encrypt input, MUGGLE_DECRYPT - decrypt input
//  * @input: input block
//  * @output: output block
//  * @ks[1|2|3]: sub key schedule 1|2|3
//  * @callbacks: DES callback functions, if NULL, use default callbacks(compatible with openssl)
//  * */
// MUGGLE_CC_EXPORT
// int muggle_tdes_crypt(
// 	int enc,
// 	const muggle_64bit_block_t *input,
// 	muggle_64bit_block_t *output,
// 	muggle_des_subkeys_t *ks1,
// 	muggle_des_subkeys_t *ks2,
// 	muggle_des_subkeys_t *ks3,
// 	muggle_des_cb_t *encrypt_cb,
// 	muggle_des_cb_t *decrypt_cb);
// 
// /*
//  * Triple DES encrypt/decrypt in mode
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
// int muggle_tdes_cipher(
// 	int mode,
// 	int enc,
// 	const unsigned char *input,
// 	unsigned char *output,
// 	unsigned int num_bytes,
// 	muggle_des_key_block key1,
// 	muggle_des_key_block key2,
// 	muggle_des_key_block key3,
// 	muggle_64bit_block_t *iv,
// 	int update_iv,
// 	muggle_des_cb_t *encrypt_cb,
// 	muggle_des_cb_t *decrypt_cb);

EXTERN_C_END

#endif
