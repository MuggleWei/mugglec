/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_INTERNAL_TDES_H_
#define MUGGLE_C_INTERNAL_TDES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"
#include "muggle/c/crypt/des.h"

EXTERN_C_BEGIN

typedef int (*fn_muggle_tdes_cipher)(
	int op,
	const muggle_des_subkeys_t *ks1,
	const muggle_des_subkeys_t *ks2,
	const muggle_des_subkeys_t *ks3,
	const unsigned char *input,
	unsigned int num_bytes,
	muggle_64bit_block_t *iv,
	int update_iv,
	unsigned char *output
);

/*
 * callbacks
 * */
int muggle_tdes_ecb(
	int op, const muggle_des_subkeys_t *ks1, const muggle_des_subkeys_t *ks2, const muggle_des_subkeys_t *ks3,
	const unsigned char *input, unsigned int num_bytes,
	muggle_64bit_block_t *iv, int update_iv, unsigned char *output);

int muggle_tdes_cbc(
	int op, const muggle_des_subkeys_t *ks1, const muggle_des_subkeys_t *ks2, const muggle_des_subkeys_t *ks3,
	const unsigned char *input, unsigned int num_bytes,
	muggle_64bit_block_t *iv, int update_iv, unsigned char *output);

int muggle_tdes_cfb(
	int op, const muggle_des_subkeys_t *ks1, const muggle_des_subkeys_t *ks2, const muggle_des_subkeys_t *ks3,
	const unsigned char *input, unsigned int num_bytes,
	muggle_64bit_block_t *iv, int update_iv, unsigned char *output);

int muggle_tdes_ofb(
	int op, const muggle_des_subkeys_t *ks1, const muggle_des_subkeys_t *ks2, const muggle_des_subkeys_t *ks3,
	const unsigned char *input, unsigned int num_bytes,
	muggle_64bit_block_t *iv, int update_iv, unsigned char *output);

int muggle_tdes_ctr(
	int op, const muggle_des_subkeys_t *ks1, const muggle_des_subkeys_t *ks2, const muggle_des_subkeys_t *ks3,
	const unsigned char *input, unsigned int num_bytes,
	muggle_64bit_block_t *iv, int update_iv, unsigned char *output);

EXTERN_C_END

#endif
