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

/*
 * DES set key
 * @param op crypt operator
 *   - MUGGLE_DECRYPT encrypt
 *   - MUGGLE_ENCRYPT decrypt
 * @param key des input key
 * @param ctx DES context
 * @return
 *   - 0 success
 *   - otherwise failed, see MUGGLE_ERR_*
 * */
static int muggle_des_set_key_inner(
	int op,
	const unsigned char key_bytes[MUGGLE_DES_BLOCK_SIZE],
	muggle_des_subkeys_t *subkeys)
{
	const muggle_64bit_block_t *key = (const muggle_64bit_block_t*)key_bytes;

#if MUGGLE_CRYPT_OPTIMIZATION
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

	return 0;
}


/*
 * DES crypt single 64bits block
 * */
static void muggle_des_crypt(
	const muggle_des_subkeys_t *ks,
	const muggle_64bit_block_t *input,
	muggle_64bit_block_t *output)
{
#if MUGGLE_CRYPT_OPTIMIZATION
	muggle_openssl_des_crypt(input, ks, output);
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
#endif
}

int muggle_des_set_key(
	int op,
	int mode,
	const unsigned char key[MUGGLE_DES_BLOCK_SIZE],
	muggle_des_context_t *ctx)
{
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(key != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	int ret = 0;

	ctx->op = op;
	ctx->mode = mode;
 	switch (mode)
 	{
 	case MUGGLE_BLOCK_CIPHER_MODE_ECB:
 	case MUGGLE_BLOCK_CIPHER_MODE_CBC:
 		{
 			ret = muggle_des_set_key_inner(op, key, &ctx->sk);
 		}break;
 	case MUGGLE_BLOCK_CIPHER_MODE_CFB:
 	case MUGGLE_BLOCK_CIPHER_MODE_OFB:
 	case MUGGLE_BLOCK_CIPHER_MODE_CTR:
 		{
 			ret = muggle_des_set_key_inner(MUGGLE_ENCRYPT, key, &ctx->sk);
 		}break;
 	default:
 		{
 			MUGGLE_ASSERT_MSG(mode >= 0 && mode < MAX_MUGGLE_BLOCK_CIPHER_MODE, "Invalid block cipher mode");
			return MUGGLE_ERR_INVALID_PARAM;
 		};
 	}

	return ret;
}

int muggle_des_ecb(
	const muggle_des_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_ECB, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ROUND_UP_POW_OF_2_MUL(num_bytes, MUGGLE_DES_BLOCK_SIZE) == num_bytes, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	unsigned int len = num_bytes / MUGGLE_DES_BLOCK_SIZE, offset = 0;
	muggle_64bit_block_t *input_block, *output_block;
	const muggle_des_subkeys_t *ks = &ctx->sk;

	for (unsigned int i = 0; i < len; ++i)
	{
		input_block = (muggle_64bit_block_t*)(input + offset);
		output_block = (muggle_64bit_block_t*)(output + offset);

		muggle_des_crypt(ks, input_block, output_block);
		offset += MUGGLE_DES_BLOCK_SIZE;
	}

	return 0;
}

int muggle_des_cbc(
	const muggle_des_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv_bytes[MUGGLE_DES_BLOCK_SIZE],
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_CBC, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ROUND_UP_POW_OF_2_MUL(num_bytes, MUGGLE_DES_BLOCK_SIZE) == num_bytes, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(iv_bytes != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	muggle_64bit_block_t *iv = (muggle_64bit_block_t*)iv_bytes;

	unsigned int len = num_bytes / MUGGLE_DES_BLOCK_SIZE, offset = 0;
	muggle_64bit_block_t v;
	v.u64 = iv->u64;
	muggle_64bit_block_t *input_block, *output_block;

	int op = ctx->op;
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);
	const muggle_des_subkeys_t *ks = &ctx->sk;

	for (unsigned int i = 0; i < len; ++i)
	{
		input_block = (muggle_64bit_block_t*)(input + offset);
		output_block = (muggle_64bit_block_t*)(output + offset);

		if (op == MUGGLE_ENCRYPT)
		{
			v.u64 ^= input_block->u64;
			muggle_des_crypt(ks, &v, output_block);
			v.u64 = output_block->u64;
		}
		else
		{
			muggle_des_crypt(ks, input_block, output_block);
			output_block->u64 ^= v.u64;
			v.u64 = input_block->u64;
		}

		offset += MUGGLE_DES_BLOCK_SIZE;
	}

	iv->u64 = v.u64;

	return 0;
}

int muggle_des_cfb64(
	const muggle_des_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE],
	unsigned int *iv_offset,
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_CFB, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(iv != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(iv_offset != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(*iv_offset < MUGGLE_DES_BLOCK_SIZE, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	int op = ctx->op;
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);
	const muggle_des_subkeys_t *ks = &ctx->sk;

	unsigned int offset = *iv_offset;
	muggle_64bit_block_t cipher_block;

	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		if (offset == 0)
		{
			muggle_des_crypt(ks, (muggle_64bit_block_t*)iv, &cipher_block);
			memcpy(iv, cipher_block.bytes, MUGGLE_DES_BLOCK_SIZE);
		}
		output[i] = input[i] ^ iv[offset];

		if (op == MUGGLE_ENCRYPT)
		{
			iv[offset] = output[i];
		}
		else
		{
			iv[offset] = input[i];
		}

		offset = (offset + 1) & 0x07;
	}

	*iv_offset = offset;

	return 0;
}

int muggle_des_ofb64(
	const muggle_des_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE],
	unsigned int *iv_offset,
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_OFB, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(iv != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(iv_offset != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(*iv_offset < MUGGLE_DES_BLOCK_SIZE, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	const muggle_des_subkeys_t *ks = &ctx->sk;

	unsigned int offset = *iv_offset;
	muggle_64bit_block_t cipher_block;

	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		if (offset == 0)
		{
			muggle_des_crypt(ks, (muggle_64bit_block_t*)iv, &cipher_block);
			memcpy(iv, cipher_block.bytes, MUGGLE_DES_BLOCK_SIZE);
		}

		output[i] = input[i] ^ iv[offset];
		offset = (offset + 1) & 0x07;
	}

	*iv_offset = offset;

	return 0;
}

int muggle_des_ctr(
	const muggle_des_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	uint64_t *nonce,
	unsigned int *nonce_offset,
	unsigned char stream_block[MUGGLE_DES_BLOCK_SIZE],
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_CTR, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(nonce != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(nonce_offset != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(*nonce_offset < MUGGLE_DES_BLOCK_SIZE, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(stream_block != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	const muggle_des_subkeys_t *ks = &ctx->sk;
	unsigned int offset = *nonce_offset;

	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		if (offset == 0)
		{
			*nonce += 1;
			muggle_des_crypt(ks, (muggle_64bit_block_t*)nonce, (muggle_64bit_block_t*)stream_block);
		}

		output[i] = input[i] ^ stream_block[offset];
		offset = (offset + 1) & 0x07;
	}

	*nonce_offset = offset;

	return 0;
}
