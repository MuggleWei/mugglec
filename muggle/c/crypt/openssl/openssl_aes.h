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

#ifndef MUGGLE_C_OPENSSL_AES_H_
#define MUGGLE_C_OPENSSL_AES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"

EXTERN_C_BEGIN

struct muggle_aes_sub_keys;

/** 
 * openssl AES_set_encrypt_key
 */
int muggle_openssl_aes_set_key(
	const unsigned char *key,
	const int bits,
	struct muggle_aes_sub_keys *sk);

/**
 * openssl AES_encrypt
 */
void muggle_openssl_aes_encrypt(
	const unsigned char *in,
	unsigned char *out,
    const struct muggle_aes_sub_keys *sk);

/**
 * openssl AES_decrypt
 */
void muggle_openssl_aes_decrypt(
	const unsigned char *in,
	unsigned char *out,
    const struct muggle_aes_sub_keys *sk);

EXTERN_C_END

#endif
