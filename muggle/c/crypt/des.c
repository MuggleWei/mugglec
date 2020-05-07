/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "des.h"
#include <stddef.h>
#include <string.h>
#include "muggle/c/base/utils.h"
#include "muggle/c/log/log.h"
#include "muggle/c/base/err.h"
#include "muggle/c/crypt/internal/internal_des.h"
#include "muggle/c/crypt/openssl/openssl_des.h"

fn_muggle_des_cipher s_fn_muggle_des[] = {
	muggle_des_ecb,
	muggle_des_cbc,
	muggle_des_cfb,
	muggle_des_ofb,
	muggle_des_ctr,
};

void muggle_des_gen_subkeys(
	int op,
	const muggle_64bit_block_t *key,
	muggle_des_subkeys_t *subkeys)
{
#if MUGGLE_CRYPT_USE_OPENSSL
	muggle_openssl_des_gen_subkeys(op, key, subkeys);
#else
	muggle_64bit_block_t out;

	// PC-1
	muggle_des_pc1(key, &out);

#if MUGGLE_CRYPT_DES_DEBUG
	printf("PC-1 C:");
	muggle_output_hex(&out.bytes[0], 4, 0);
	muggle_output_bin(&out.bytes[0], 4, 8);
	printf("PC-1 D:");
	muggle_output_hex(&out.bytes[4], 4, 0);
	muggle_output_bin(&out.bytes[4], 4, 8);
#endif

	const int key_round_shift[16] = {
		1, 1, 2, 2,
		2, 2, 2, 2,
		1, 2, 2, 2,
		2, 2, 2, 1
	};
	const int key_round_mask[16] = {
		0x01, 0x01, 0x03, 0x03,
		0x03, 0x03, 0x03, 0x03,
		0x01, 0x03, 0x03, 0x03,
		0x03, 0x03, 0x03, 0x01
	};

	for (int i = 0; i < 16; i++)
	{
		// shift
		out.u32.l = MUGGLE_DES_KEY_SHIFT(out.u32.l, key_round_shift[i], key_round_mask[i]);
		out.u32.h = MUGGLE_DES_KEY_SHIFT(out.u32.h, key_round_shift[i], key_round_mask[i]);

		// PC-2
		int idx = i;
		if (op == MUGGLE_DECRYPT)
		{
			idx = 15 - i;
		}
		muggle_des_pc2(&out, &subkeys->sk[idx]);
	}
#endif
}


int muggle_des_crypt(
	const muggle_64bit_block_t *input,
	const muggle_des_subkeys_t *ks,
	muggle_64bit_block_t *output)
{
#if MUGGLE_CRYPT_USE_OPENSSL
	return muggle_openssl_des_crypt(input, ks, output);
#else
	muggle_64bit_block_t b64;
	muggle_32bit_block_t b32;
	uint32_t tmp;

	// IP
	muggle_des_ip(input, &b64);

#if MUGGLE_CRYPT_DES_DEBUG
	printf("IP: ");
	muggle_output_hex(b64.bytes, 8, 0);
	muggle_output_bin(b64.bytes, 8, 8);
#endif

	// Feistel, 16 rounds
	for (int i = 0; i < 16; ++i)
	{
#if MUGGLE_CRYPT_DES_DEBUG
		printf("feistel[%d]: ", i);
		muggle_output_hex(b64.bytes, 8, 0);
#endif

		muggle_32bit_block_t *r = (muggle_32bit_block_t*)&b64.u32.h;
		muggle_des_f(r, &ks->sk[i], &b32);
		tmp = b64.u32.l ^ b32.u32;
		b64.u32.l = b64.u32.h;
		b64.u32.h = tmp;
	}

#if MUGGLE_CRYPT_DES_DEBUG
	printf("feistel[16]: ");
	muggle_output_hex(b64.bytes, 8, 0);
#endif

	// FP
	tmp = b64.u32.l;
	b64.u32.l = b64.u32.h;
	b64.u32.h = tmp;

	muggle_des_fp(&b64, output);

	return 0;
#endif
}

int muggle_des_cipher_bytes(
	int op,
	int block_cipher_mode,
	muggle_64bit_block_t key,
	const unsigned char *input,
	unsigned int num_bytes,
	muggle_64bit_block_t *iv,
	int update_iv,
	unsigned char *output)
{
	muggle_des_subkeys_t ks;
	switch (block_cipher_mode)
	{
	case MUGGLE_BLOCK_CIPHER_MODE_ECB:
	case MUGGLE_BLOCK_CIPHER_MODE_CBC:
		{
			muggle_des_gen_subkeys(op, &key, &ks);
		}break;
	case MUGGLE_BLOCK_CIPHER_MODE_CFB:
	case MUGGLE_BLOCK_CIPHER_MODE_OFB:
	case MUGGLE_BLOCK_CIPHER_MODE_CTR:
		{
			muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key, &ks);
		}break;
	default:
		{
			MUGGLE_ASSERT_MSG(block_cipher_mode >= 0 && block_cipher_mode < MAX_MUGGLE_BLOCK_CIPHER_MODE, "Invalid block cipher mode");
			return MUGGLE_ERR_INVALID_PARAM;
		};
	}

	return muggle_des_cipher(op, block_cipher_mode, &ks, input, num_bytes, iv, update_iv, output);
}
 
int muggle_des_cipher(
	int op,
	int block_cipher_mode,
	const muggle_des_subkeys_t *ks,
	const unsigned char *input,
	unsigned int num_bytes,
	muggle_64bit_block_t *iv,
	int update_iv,
	unsigned char *output)
{
	if (input == NULL)
	{
		MUGGLE_ASSERT_MSG(input != NULL, "DES cipher failed: input is nullptr");
		return MUGGLE_ERR_NULL_PARAM;
	}

	if (ROUND_UP_POW_OF_2_MUL(num_bytes, 8) != num_bytes)
	{
		MUGGLE_ASSERT_MSG(ROUND_UP_POW_OF_2_MUL(num_bytes, 8) == num_bytes, "DES cipher failed: input bytes is not multiple of 8");
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (update_iv != 0 && iv == NULL)
	{
		MUGGLE_ASSERT_MSG(!(update_iv != 0 && iv == NULL), "Invalid! iv is null and wanna update iv");
		return MUGGLE_ERR_INVALID_PARAM;
	}

	return s_fn_muggle_des[block_cipher_mode](op, ks, input, num_bytes, iv, update_iv, output);
}
