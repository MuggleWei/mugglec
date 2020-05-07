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

/*
 * Triple DES encrypt/decrypt a single 64bit block
 * @param input input block
 * @ks[1|2|3]: sub key schedule 1|2|3
 * @output: output block
 * @callbacks: DES callback functions, if NULL, use default callbacks(compatible with openssl)
 * */
MUGGLE_CC_EXPORT
int muggle_tdes_crypt(
	const muggle_64bit_block_t *input,
	const muggle_des_subkeys_t *ks1,
	const muggle_des_subkeys_t *ks2,
	const muggle_des_subkeys_t *ks3,
	muggle_64bit_block_t *output);

/*
 * Triple DES encrypt/decrypt in mode
 * @param op
 *   - MUGGLE_DECRYPT encrypt
 *   - MUGGLE_ENCRYPT decrypt
 * @param block_cipher_mode block cipher mode, use MUGGLE_BLOCK_CIPHER_MODE_*
 * @param key[1|2|3] tdes input keys
 * @input: plaintext bytes
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector;
 * 	ECB: doesn't make sense in ECB mode
 * 	CTR: equal to init counter values
 * @param update_iv 1 - update, 0 - don't update;
 *  ECB: doesn't make sense in ECB mode
 *  CTR: whether CTR need update counter values
 * @param output output bytes
 * @return return 0 on success, otherwise failed
 *
 * NOTE: Don't use ECB mode in product environment
 * */
MUGGLE_CC_EXPORT
int muggle_tdes_cipher_bytes(
	int op,
	int block_cipher_mode,
	muggle_64bit_block_t key1,
	muggle_64bit_block_t key2,
	muggle_64bit_block_t key3,
	const unsigned char *input,
	unsigned int num_bytes,
	muggle_64bit_block_t *iv,
	int update_iv,
	unsigned char *output);

/*
 * Triple DES encrypt/decrypt in mode
 * @param op
 *   - MUGGLE_DECRYPT encrypt
 *   - MUGGLE_ENCRYPT decrypt
 * @param block_cipher_mode block cipher mode, use MUGGLE_BLOCK_CIPHER_MODE_*
 * @param ks[1|2|3] tdes input key schedules
 * @input: plaintext bytes
 * @param num_bytes length of input/output bytes
 * @param iv initialization vector;
 * 	ECB: doesn't make sense in ECB mode
 * 	CTR: equal to init counter values
 * @param update_iv 1 - update, 0 - don't update;
 *  ECB: doesn't make sense in ECB mode
 *  CTR: whether CTR need update counter values
 * @param output output bytes
 * @return return 0 on success, otherwise failed
 *
 * NOTE: Don't use ECB mode in product environment
 * */
MUGGLE_CC_EXPORT
int muggle_tdes_cipher(
	int op,
	int block_cipher_mode,
	const muggle_des_subkeys_t *ks1,
	const muggle_des_subkeys_t *ks2,
	const muggle_des_subkeys_t *ks3,
	const unsigned char *input,
	unsigned int num_bytes,
	muggle_64bit_block_t *iv,
	int update_iv,
	unsigned char *output);

EXTERN_C_END

#endif
