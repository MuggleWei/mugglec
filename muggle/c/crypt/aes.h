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

#define MUGGLE_AES_MAX_NUM_ROUND 14

typedef struct muggle_aes_sub_keys
{
	uint32_t rd_key[4 * (MUGGLE_AES_MAX_NUM_ROUND + 1)];
	int rounds;
}muggle_aes_sub_keys_t;

/*
 * generate AES round keys
 * @param key user input key
 * @param bits number of bit of key (128|192|256)
 * @param sk output AES round keys
 * @return
 * - 0 success
 * - MUGGLE_ERR_CRYPT_KEY_SIZE invalid key size
 * */
MUGGLE_CC_EXPORT
int muggle_aes_key_setup(const unsigned char *key, int bits, muggle_aes_sub_keys_t *sk);

/*
 * Encrypt/Decrypt a single block
 * @param mode
 * - MUGGLE_ENCRYPT encrypt
 * - MUGGLE_DECRYPT decrypt
 * @param input input single block
 * @param sk key schedule
 * @param output output single block
 * */
MUGGLE_CC_EXPORT
int muggle_aes_crypt(
	int mode,
	const unsigned char *input,
	muggle_aes_sub_keys_t *sk,
	unsigned char *output);

EXTERN_C_END

#endif
