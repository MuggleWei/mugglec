/******************************************************************************
 *  @file         tdes.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec crypt TDES
 *****************************************************************************/
 
#include "tdes.h"
#include <stddef.h>
#include <string.h>
#include "muggle/c/base/utils.h"
#include "muggle/c/log/log.h"
#include "muggle/c/base/err.h"
#include "muggle/c/crypt/internal/internal_des.h"
#include "muggle/c/crypt/openssl/openssl_des.h"

/*
 * TDES crypt single 64bits block
 * */
static void muggle_tdes_crypt(
	const muggle_des_subkeys_t *ks1,
	const muggle_des_subkeys_t *ks2,
	const muggle_des_subkeys_t *ks3,
	const muggle_64bit_block_t *input,
	muggle_64bit_block_t *output)
{
#if MUGGLE_CRYPT_OPTIMIZATION
	muggle_openssl_tdes_crypt(input, ks1, ks2, ks3, output);
#else
	muggle_64bit_block_t b64;
	muggle_32bit_block_t b32;
	uint32_t tmp;

	// IP
	muggle_des_ip(input, &b64);

	// Feistel, 16 rounds
	for (int i = 0; i < 16; ++i)
	{
		muggle_32bit_block_t *r = (muggle_32bit_block_t*)&b64.u32.h;
		muggle_des_f(r, &ks1->sk[i], &b32);
		tmp = b64.u32.l ^ b32.u32;
		b64.u32.l = b64.u32.h;
		b64.u32.h = tmp;
	}

	for (int i = 0; i < 16; ++i)
	{
		muggle_32bit_block_t *r = (muggle_32bit_block_t*)&b64.u32.h;
		muggle_des_f(r, &ks2->sk[i], &b32);
		tmp = b64.u32.l ^ b32.u32;
		b64.u32.l = b64.u32.h;
		b64.u32.h = tmp;
	}

	for (int i = 0; i < 16; ++i)
	{
		muggle_32bit_block_t *r = (muggle_32bit_block_t*)&b64.u32.h;
		muggle_des_f(r, &ks3->sk[i], &b32);
		tmp = b64.u32.l ^ b32.u32;
		b64.u32.l = b64.u32.h;
		b64.u32.h = tmp;
	}

	// FP
	tmp = b64.u32.l;
	b64.u32.l = b64.u32.h;
	b64.u32.h = tmp;

	muggle_des_fp(&b64, output);

#endif
}

int muggle_tdes_set_key(
	int op,
	int mode,
	const unsigned char key1[MUGGLE_DES_BLOCK_SIZE],
	const unsigned char key2[MUGGLE_DES_BLOCK_SIZE],
	const unsigned char key3[MUGGLE_DES_BLOCK_SIZE],
	muggle_tdes_context_t *ctx)
{
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(key1 != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(key2 != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(key3 != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	int inv_op = op == MUGGLE_ENCRYPT ? MUGGLE_DECRYPT : MUGGLE_ENCRYPT;

	int ret = 0;

	ctx->op = op;
	ctx->mode = mode;
	switch (mode)
	{
	case MUGGLE_BLOCK_CIPHER_MODE_ECB:
	case MUGGLE_BLOCK_CIPHER_MODE_CBC:
		{
			mode = MUGGLE_BLOCK_CIPHER_MODE_ECB;
			if (op == MUGGLE_ENCRYPT)
			{
				ret = muggle_des_set_key(op, mode, key1, &ctx->ctx1);
				MUGGLE_CHECK_RET(ret == 0, ret);

				ret = muggle_des_set_key(inv_op, mode, key2, &ctx->ctx2);
				MUGGLE_CHECK_RET(ret == 0, ret);

				ret = muggle_des_set_key(op, mode, key3, &ctx->ctx3);
				MUGGLE_CHECK_RET(ret == 0, ret);
			}
			else
			{
				ret = muggle_des_set_key(op, mode, key3, &ctx->ctx1);
				MUGGLE_CHECK_RET(ret == 0, ret);

				ret = muggle_des_set_key(inv_op, mode, key2, &ctx->ctx2);
				MUGGLE_CHECK_RET(ret == 0, ret);

				ret = muggle_des_set_key(op, mode, key1, &ctx->ctx3);
				MUGGLE_CHECK_RET(ret == 0, ret);
			}
		}break;
	case MUGGLE_BLOCK_CIPHER_MODE_CFB:
	case MUGGLE_BLOCK_CIPHER_MODE_OFB:
	case MUGGLE_BLOCK_CIPHER_MODE_CTR:
		{
			mode = MUGGLE_BLOCK_CIPHER_MODE_ECB;

			ret = muggle_des_set_key(MUGGLE_ENCRYPT, mode, key1, &ctx->ctx1);
			MUGGLE_CHECK_RET(ret == 0, ret);

			ret = muggle_des_set_key(MUGGLE_DECRYPT, mode, key2, &ctx->ctx2);
			MUGGLE_CHECK_RET(ret == 0, ret);

			ret = muggle_des_set_key(MUGGLE_ENCRYPT, mode, key3, &ctx->ctx3);
			MUGGLE_CHECK_RET(ret == 0, ret);
		}break;
	default:
		{
			MUGGLE_ASSERT_MSG(mode >= 0 && mode < MAX_MUGGLE_BLOCK_CIPHER_MODE, "Invalid block cipher mode");
			return MUGGLE_ERR_INVALID_PARAM;
		};
 	}

	return ret;
}

int muggle_tdes_ecb(
	muggle_tdes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_ECB, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(MUGGLE_ROUND_UP_POW_OF_2_MUL(num_bytes, MUGGLE_DES_BLOCK_SIZE) == num_bytes, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	size_t len = num_bytes / 8, offset = 0;
	muggle_64bit_block_t *input_block, *output_block;
	const muggle_des_subkeys_t *ks1 = &ctx->ctx1.sk;
	const muggle_des_subkeys_t *ks2 = &ctx->ctx2.sk;
	const muggle_des_subkeys_t *ks3 = &ctx->ctx3.sk;

	for (size_t i = 0; i < len; ++i)
	{
		input_block = (muggle_64bit_block_t*)(input + offset);
		output_block = (muggle_64bit_block_t*)(output + offset);

		muggle_tdes_crypt(ks1, ks2, ks3, input_block, output_block);
		offset += 8;
	}

	return 0;
}

int muggle_tdes_cbc(
	const muggle_tdes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv_bytes[MUGGLE_DES_BLOCK_SIZE],
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_CBC, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(MUGGLE_ROUND_UP_POW_OF_2_MUL(num_bytes, MUGGLE_DES_BLOCK_SIZE) == num_bytes, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(iv_bytes != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	muggle_64bit_block_t *iv = (muggle_64bit_block_t*)iv_bytes;
	muggle_64bit_block_t v;
	v.u64 = iv->u64;

	unsigned int len = num_bytes / 8, offset = 0;
	muggle_64bit_block_t *input_block, *output_block;
	const muggle_des_subkeys_t *ks1 = &ctx->ctx1.sk;
	const muggle_des_subkeys_t *ks2 = &ctx->ctx2.sk;
	const muggle_des_subkeys_t *ks3 = &ctx->ctx3.sk;

	int op = ctx->op;
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);
	for (size_t i = 0; i < len; ++i)
	{
		input_block = (muggle_64bit_block_t*)(input + offset);
		output_block = (muggle_64bit_block_t*)(output + offset);

		if (op == MUGGLE_ENCRYPT)
		{
			v.u64 ^= input_block->u64;
			muggle_tdes_crypt(ks1, ks2, ks3, &v, output_block);
			v.u64 = output_block->u64;
		}
		else
		{
			muggle_tdes_crypt(ks1, ks2, ks3, input_block, output_block);
			output_block->u64 ^= v.u64;
			v.u64 = input_block->u64;
		}

		offset += 8;
	}

	iv->u64 = v.u64;

	return 0;
}

int muggle_tdes_cfb64(
	const muggle_tdes_context_t *ctx,
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
	const muggle_des_subkeys_t *ks1 = &ctx->ctx1.sk;
	const muggle_des_subkeys_t *ks2 = &ctx->ctx2.sk;
	const muggle_des_subkeys_t *ks3 = &ctx->ctx3.sk;

	unsigned int offset = *iv_offset;
	muggle_64bit_block_t cipher_block;

	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		if (offset == 0)
		{
			muggle_tdes_crypt(ks1, ks2, ks3, (muggle_64bit_block_t*)iv, &cipher_block);
			memcpy(iv, cipher_block.bytes, 8);
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

int muggle_tdes_ofb64(
	const muggle_tdes_context_t *ctx,
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

	const muggle_des_subkeys_t *ks1 = &ctx->ctx1.sk;
	const muggle_des_subkeys_t *ks2 = &ctx->ctx2.sk;
	const muggle_des_subkeys_t *ks3 = &ctx->ctx3.sk;

	unsigned int offset = *iv_offset;
	muggle_64bit_block_t cipher_block;

	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		if (offset == 0)
		{
			muggle_tdes_crypt(ks1, ks2, ks3, (muggle_64bit_block_t*)iv, &cipher_block);
			memcpy(iv, cipher_block.bytes, 8);
		}

		output[i] = input[i] ^ iv[offset];
		offset = (offset + 1) & 0x07;
	}

	*iv_offset = offset;

	return 0;
}

int muggle_tdes_ctr(
	const muggle_tdes_context_t *ctx,
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

	const muggle_des_subkeys_t *ks1 = &ctx->ctx1.sk;
	const muggle_des_subkeys_t *ks2 = &ctx->ctx2.sk;
	const muggle_des_subkeys_t *ks3 = &ctx->ctx3.sk;
	unsigned int offset = *nonce_offset;

	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		if (offset == 0)
		{
			*nonce += 1;
			muggle_tdes_crypt(ks1, ks2, ks3, (muggle_64bit_block_t*)nonce, (muggle_64bit_block_t*)stream_block);
		}

		output[i] = input[i] ^ stream_block[offset];
		offset = (offset + 1) & 0x07;
	}

	*nonce_offset = offset;

	return 0;
}
