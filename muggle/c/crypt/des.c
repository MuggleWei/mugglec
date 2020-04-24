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
#include "muggle/c/crypt/internal/internal_des.h"

// // callbacks
// int muggle_des_ecb(
// 	int enc, const unsigned char *input, unsigned char *output, unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks, muggle_64bit_block_t *iv,	int update_iv, muggle_des_cb_t *callbacks);
// int muggle_des_cbc(
// 	int enc, const unsigned char *input, unsigned char *output, unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks, muggle_64bit_block_t *iv,	int update_iv, muggle_des_cb_t *callbacks);
// int muggle_des_cfb(
// 	int enc, const unsigned char *input, unsigned char *output, unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks, muggle_64bit_block_t *iv,	int update_iv, muggle_des_cb_t *callbacks);
// int muggle_des_ofb(
// 	int enc, const unsigned char *input, unsigned char *output, unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks, muggle_64bit_block_t *iv,	int update_iv, muggle_des_cb_t *callbacks);
// int muggle_des_ctr(
// 	int enc, const unsigned char *input, unsigned char *output, unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks, muggle_64bit_block_t *iv,	int update_iv, muggle_des_cb_t *callbacks);
// 
// fn_muggle_des_cipher s_fn_muggle_des[] = {
// 	muggle_des_ecb,
// 	muggle_des_cbc,
// 	muggle_des_cfb,
// 	muggle_des_ofb,
// 	muggle_des_ctr,
// };

void muggle_des_gen_subkeys(
	int mode,
	const muggle_64bit_block_t *key,
	muggle_des_subkeys_t *subkeys)
{
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
		out.u32.l = DES_KEY_SHIFT(out.u32.l, key_round_shift[i], key_round_mask[i]);
		out.u32.h = DES_KEY_SHIFT(out.u32.h, key_round_shift[i], key_round_mask[i]);

		// PC-2
		int idx = i;
		if (mode == MUGGLE_DECRYPT)
		{
			idx = 15 - i;
		}
		muggle_des_pc2(&out, &subkeys->sk[idx]);
	}
}


int muggle_des_crypt(
	const muggle_64bit_block_t *input,
	const muggle_des_subkeys_t *ks,
	muggle_64bit_block_t *output)
{
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
}

// int muggle_des_cipher(
// 	int mode,
// 	int enc,
// 	const unsigned char *input,
// 	unsigned char *output,
// 	unsigned int num_bytes,
// 	muggle_des_key_block key,
// 	muggle_64bit_block_t *iv,
// 	int update_iv,
// 	muggle_des_cb_t *callbacks)
// {
// 	if (mode < 0 || mode >= MAX_MUGGLE_BLOCK_CIPHER_MODE)
// 	{
// 		MUGGLE_LOG_ERROR("invalid block cipher mode: %d", mode);
// 		return -1;
// 	}
// 
// 	if (ROUND_UP_POW_OF_2_MUL(num_bytes, 8) != num_bytes)
// 	{
// 		MUGGLE_LOG_ERROR("DES cipher failed: input bytes is not multiple of 8");
// 		return -1;
// 	}
// 	size_t len = num_bytes / 8;
// 
// 	// generate subkey
// 	int ret = 0;
// 	muggle_des_subkeys_t ks;
// 	if (callbacks == NULL)
// 	{
// 		ret = des_default_gen_subkey(&key, &ks);
// 	}
// 	else
// 	{
// 		ret = callbacks->gen_subkey(&key, &ks);
// 	}
// 
// 	if (ret != 0)
// 	{
// 		MUGGLE_LOG_ERROR("DES cipher failed, failed gen subkey");
// 		return -1;
// 	}
// 
// 	return s_fn_muggle_des[mode](enc, input, output, num_bytes, &ks, iv, update_iv, callbacks);
// }
// 
// int muggle_des_ecb(
// 	int enc, const unsigned char *input, unsigned char *output, unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks, muggle_64bit_block_t *iv,	int update_iv, muggle_des_cb_t *callbacks)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t *input_block, *output_block;
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		ret = muggle_des_crypt(enc, input_block, output_block, ks, callbacks);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("DES ECB crypt failed");
// 			return ret;
// 		}
// 		offset += 8;
// 	}
// 
// 	return 0;
// }
// 
// int muggle_des_cbc(
// 	int enc, const unsigned char *input, unsigned char *output, unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks, muggle_64bit_block_t *iv,	int update_iv, muggle_des_cb_t *callbacks)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t v;
// 	v.u64 = iv->u64;
// 	muggle_64bit_block_t *input_block, *output_block;
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		if (enc)
// 		{
// 			v.u64 ^= input_block->u64;
// 			ret = muggle_des_crypt(enc, &v, output_block, ks, callbacks);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("DES ECB crypt failed");
// 				return ret;
// 			}
// 
// 			v.u64 = output_block->u64;
// 		}
// 		else
// 		{
// 			ret = muggle_des_crypt(enc, input_block, output_block, ks, callbacks);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("DES ECB crypt failed");
// 				return ret;
// 			}
// 			output_block->u64 ^= v.u64;
// 
// 			v.u64 = input_block->u64;
// 		}
// 
// 		offset += 8;
// 	}
// 
// 	if (update_iv)
// 	{
// 		iv->u64 = v.u64;
// 	}
// 
// 	return 0;
// }
// 
// int muggle_des_cfb(
// 	int enc, const unsigned char *input, unsigned char *output, unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks, muggle_64bit_block_t *iv,	int update_iv, muggle_des_cb_t *callbacks)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t v;
// 	v.u64 = iv->u64;
// 	muggle_64bit_block_t *input_block, *output_block;
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		if (enc)
// 		{
// 			ret = muggle_des_crypt(enc, &v, output_block, ks, callbacks);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("DES CFB crypt failed");
// 				return ret;
// 			}
// 			output_block->u64 ^= input_block->u64;
// 			v.u64 = output_block->u64;
// 		}
// 		else
// 		{
// 			ret = muggle_des_crypt(!enc, &v, output_block, ks, callbacks);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("DES CFB crypt failed");
// 				return ret;
// 			}
// 			output_block->u64 ^= input_block->u64;
// 			v.u64 = input_block->u64;
// 		}
// 
// 		offset += 8;
// 	}
// 
// 	if (update_iv)
// 	{
// 		iv->u64 = v.u64;
// 	}
// 
// 	return 0;
// }
// 
// int muggle_des_ofb(
// 	int enc, const unsigned char *input, unsigned char *output, unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks, muggle_64bit_block_t *iv,	int update_iv, muggle_des_cb_t *callbacks)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t v;
// 	v.u64 = iv->u64;
// 	muggle_64bit_block_t *input_block, *output_block;
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		if (enc)
// 		{
// 			ret = muggle_des_crypt(enc, &v, output_block, ks, callbacks);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("DES OFB crypt failed");
// 				return ret;
// 			}
// 			v.u64 = output_block->u64;
// 			output_block->u64 ^= input_block->u64;
// 		}
// 		else
// 		{
// 			ret = muggle_des_crypt(!enc, &v, output_block, ks, callbacks);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("DES OFB crypt failed");
// 				return ret;
// 			}
// 			v.u64 = output_block->u64;
// 			output_block->u64 ^= input_block->u64;
// 		}
// 
// 		offset += 8;
// 	}
// 
// 	if (update_iv)
// 	{
// 		iv->u64 = v.u64;
// 	}
// 
// 	return 0;
// }
// 
// int muggle_des_ctr(
// 	int enc, const unsigned char *input, unsigned char *output, unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks, muggle_64bit_block_t *iv,	int update_iv, muggle_des_cb_t *callbacks)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t v;
// 	v.u64 = iv->u64;
// 	muggle_64bit_block_t *input_block, *output_block;
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		if (enc)
// 		{
// 			ret = muggle_des_crypt(enc, &v, output_block, ks, callbacks);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("DES CTR crypt failed");
// 				return ret;
// 			}
// 			output_block->u64 ^= input_block->u64;
// 			v.u64++;
// 		}
// 		else
// 		{
// 			ret = muggle_des_crypt(!enc, &v, output_block, ks, callbacks);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("DES CTR crypt failed");
// 				return ret;
// 			}
// 			output_block->u64 ^= input_block->u64;
// 			v.u64++;
// 		}
// 
// 		offset += 8;
// 	}
// 
// 	if (update_iv)
// 	{
// 		iv->u64 = v.u64;
// 	}
// 
// 	return 0;
// }

