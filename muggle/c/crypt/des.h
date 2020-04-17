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
 * DES Initial Permutation
 * @in: input block
 * @out: output block
 *
 * permutation table:
 *   58  50  42  34  26  18  10   2
 *   60  52  44  36  28  20  12   4
 *   62  54  46  38  30  22  14   6
 *   64  56  48  40  32  24  16   8
 *   57  49  41  33  25  17   9   1
 *   59  51  43  35  27  19  11   3
 *   61  53  45  37  29  21  13   5
 *   63  55  47  39  31  23  15   7
 *
 * */
MUGGLE_CC_EXPORT
void muggle_des_ip(const muggle_64bit_block_t *in, muggle_64bit_block_t *out);

/*
 * DES encrypt/decrypt a single 64bit block
 * @enc: MUGGLE_ENCRYPT - encrypt input, MUGGLE_DECRYPT - decrypt input
 * @input: input block
 * @output: output block
 * @ks: sub key schedule
 * @callbacks: DES callback functions, if NULL, use default callbacks(compatible with openssl)
 * */
MUGGLE_CC_EXPORT
int muggle_des_crypt(
	int enc,
	const muggle_64bit_block_t *input,
	muggle_64bit_block_t *output,
	muggle_des_subkeys_t *ks,
	muggle_des_cb_t *callbacks);

/*
 * DES encrypt/decrypt in mode
 * @mode: block cipher mode, use MUGGLE_BLOCK_CIPHER_MODE_*
 * @enc: MUGGLE_ENCRYPT - encrypt input, MUGGLE_DECRYPT - decrypt input
 * @input: plaintext bytes
 * @output: ciphertext bytes
 * @num_bytes: length of input/output bytes
 * @key: key
 * @iv: initialization vector;
 * 	ECB: doesn't make sense in ECB mode
 * 	CTR: equal to init counter values
 * @update_iv: 1 - update, 0 - don't update;
 *  ECB: doesn't make sense in ECB mode
 *  CTR: whether CTR need update counter values
 * @callbacks: if NULL, use default callbacks
 * RETURN: 
 *  return 0 on success, otherwise failed
 *
 * NOTE: Don't use ECB mode in product environment
 * */
MUGGLE_CC_EXPORT
int muggle_des_cipher(
	int mode,
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
