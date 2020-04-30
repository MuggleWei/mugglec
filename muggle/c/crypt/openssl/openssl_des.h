/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

/*
 * Compatibility with openssl
 *
 * this header file and the corresponding source file are
 * all copied and modified from openssl(Apache License v2.0)
 *
 * */

#ifndef MUGGLE_C_OPENSSL_DES_H_
#define MUGGLE_C_OPENSSL_DES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"

EXTERN_C_BEGIN

struct muggle_des_subkeys;

/**
 * DES key schedule
 * @param op encryption or decryption, use MUGGLE_DECRYPT or MUGGLE_ENCRYPT
 * @param key des input key
 * @param subkeys output subkeys
 * */
void muggle_openssl_des_gen_subkeys(
	int op,
	const muggle_64bit_block_t *key,
	struct muggle_des_subkeys *subkeys);

/*
 * DES encrypt/decrypt a single 64bit block
 * @param input input block
 * @param ks sub key schedule
 * @param output output block
 * @return 0 represents success, otherwise failed
 * */
int muggle_openssl_des_crypt(
	const muggle_64bit_block_t *input,
	const struct muggle_des_subkeys *ks,
	muggle_64bit_block_t *output);

EXTERN_C_END

#endif
