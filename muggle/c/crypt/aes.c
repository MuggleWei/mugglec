/******************************************************************************
 *  @file         aes.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec crypt AES
 *****************************************************************************/
 
#include "aes.h"
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"
#include "muggle/c/log/log.h"
#include "muggle/c/crypt/internal/internal_aes.h"
#include "muggle/c/crypt/openssl/openssl_aes.h"

static int muggle_aes_crypt(
	int op,
	const unsigned char *input,
	const muggle_aes_subkeys_t *sk,
	unsigned char *output)
{
#if MUGGLE_CRYPT_OPTIMIZATION
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);

	if (op == MUGGLE_ENCRYPT)
	{
		muggle_openssl_aes_encrypt(input, output, sk);
	}
	else
	{
		muggle_openssl_aes_decrypt(input, output, sk);
	}

	return 0;
#else
	int ret = 0;
	unsigned char state[16] = {
		input[0], input[4], input[8],  input[12],
		input[1], input[5], input[9],  input[13],
		input[2], input[6], input[10], input[14],
		input[3], input[7], input[11], input[15]
	};

	if (op == MUGGLE_ENCRYPT)
	{
		ret = muggle_aes_encrypt(state, sk);
	}
	else if (op == MUGGLE_DECRYPT)
	{
		ret = muggle_aes_decrypt(state, sk);
	}
	else
	{
		MUGGLE_ASSERT_MSG(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, "Invalid AES crypt mode");
		ret = MUGGLE_ERR_INVALID_PARAM;
	}

	output[0]  = state[0];
	output[1]  = state[4];
	output[2]  = state[8];
	output[3]  = state[12];
	output[4]  = state[1];
	output[5]  = state[5];
	output[6]  = state[9];
	output[7]  = state[13];
	output[8]  = state[2];
	output[9]  = state[6];
	output[10] = state[10];
	output[11] = state[14];
	output[12] = state[3];
	output[13] = state[7];
	output[14] = state[11];
	output[15] = state[15];

	return ret;
#endif
}

int muggle_aes_set_key(
	int op,
	int mode,
	const unsigned char *key,
	int bits,
	muggle_aes_context_t *ctx)
{
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(mode >= MUGGLE_BLOCK_CIPHER_MODE_ECB && mode < MAX_MUGGLE_BLOCK_CIPHER_MODE, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(key != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);

	ctx->op = op;
	ctx->mode = mode;

#if MUGGLE_CRYPT_OPTIMIZATION
	return muggle_openssl_aes_set_key(key, bits, &ctx->sk);
#else
	// variable's name is consistent with fips-197
	int Nb, Nr, Nk, i;

	Nb = 4;
	switch (bits)
	{
	case 128: 
	{
		Nr = 10;
		Nk = 4;
	}break;
	case 192:
	{
		Nr = 12;
		Nk = 6;
	}break;
	case 256:
	{
		Nr = 14;
		Nk = 8;
	}break;
	default:
	{
		MUGGLE_ASSERT_MSG(
			bits == 128 || bits == 192 || bits == 256,
			"AES key setup only support bit size: 128/192/256");
		return MUGGLE_ERR_CRYPT_KEY_SIZE;
	}
	}

	muggle_aes_subkeys_t *sk = &ctx->sk;
	i = 0;
	sk->rounds = Nr;
	for (i = 0; i < Nk; i++)
	{
		sk->rd_key[i] =
			(key[4*i + 0] << 24) |
			(key[4*i + 1] << 16) |
			(key[4*i + 2] << 8) |
			(key[4*i + 3]);
	}

	for (i = Nk; i < Nb * (Nr + 1); i++)
	{
		uint32_t temp = sk->rd_key[i - 1];
#if MUGGLE_CRYPT_AES_DEBUG
		printf("Expansion key[%d]: temp=%08x\n", i, temp);
#endif
		if (i % Nk == 0)
		{
			// RotWord
			temp = muggle_aes_rot_word(temp);
#if MUGGLE_CRYPT_AES_DEBUG
			printf("Expansion key[%d]: After RotWord=%08x\n", i, temp);
#endif

			// SubWord
			temp = muggle_aes_sub_word(temp);
#if MUGGLE_CRYPT_AES_DEBUG
			printf("Expansion key[%d]: After SubWord=%08x\n", i, temp);
			printf("Expansion key[%d]: Rcon[i/Nk]=%08x\n", i, s_aes_rcon[(i-1)/Nk]); 
#endif

			// w[i]
			temp = temp ^ s_aes_rcon[(i-1)/Nk];

#if MUGGLE_CRYPT_AES_DEBUG
			printf("Expansion key[%d]: After XOR with Rcon=%08x\n", i, temp);
#endif
		}
		else if (Nk > 6 && (i % Nk == 4))
		{
			temp = muggle_aes_sub_word(temp);
#if MUGGLE_CRYPT_AES_DEBUG
			printf("Expansion key[%d]: After SubWord=%08x\n", i, temp);
#endif
		}

#if MUGGLE_CRYPT_AES_DEBUG
		printf("Expansion key[%d]: w[i-Nk]=%08x\n", i, sk->rd_key[i-Nk]);
#endif

		sk->rd_key[i] = sk->rd_key[i-Nk] ^ temp;

#if MUGGLE_CRYPT_AES_DEBUG
		printf("Expansion key[%d]: w[i]=%08x\n", i, sk->rd_key[i]);
#endif
	}

	return 0;
#endif
}

int muggle_aes_ecb(
	const muggle_aes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_ECB, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ROUND_UP_POW_OF_2_MUL(num_bytes, MUGGLE_AES_BLOCK_SIZE) == num_bytes, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	const unsigned char *input_block;
	unsigned char *output_block;
	unsigned int len = num_bytes / MUGGLE_AES_BLOCK_SIZE, offset = 0;
	int op = ctx->op;
	const muggle_aes_subkeys_t *sk = &ctx->sk;
	for (unsigned int i = 0; i < len; ++i)
	{
		input_block = input + offset;
		output_block = output + offset;

		muggle_aes_crypt(op, input_block, sk, output_block);
		offset += MUGGLE_AES_BLOCK_SIZE;
	}

	return 0;
}

static void aes_128bit_xor(uint64_t *v1, uint64_t *v2)
{
	v1[0] ^= v2[0];
	v1[1] ^= v2[1];
}

int muggle_aes_cbc(
	const muggle_aes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_AES_BLOCK_SIZE],
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_CBC, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ROUND_UP_POW_OF_2_MUL(num_bytes, MUGGLE_AES_BLOCK_SIZE) == num_bytes, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(iv != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	const unsigned char *input_block;
	unsigned char *output_block;
	unsigned int len = num_bytes / MUGGLE_AES_BLOCK_SIZE, offset = 0;
	int op = ctx->op;
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);
	const muggle_aes_subkeys_t *sk = &ctx->sk;

	for (unsigned int i = 0; i < len; ++i)
	{
		input_block = input + offset;
		output_block = output + offset;

		if (op == MUGGLE_ENCRYPT)
		{
			aes_128bit_xor((uint64_t*)iv, (uint64_t*)input_block);
			muggle_aes_crypt(op, iv, sk, output_block);
			memcpy(iv, output_block, MUGGLE_AES_BLOCK_SIZE);
		}
		else
		{
			muggle_aes_crypt(op, input_block, sk, output_block);
			aes_128bit_xor((uint64_t*)output_block, (uint64_t*)iv);
			memcpy(iv, input_block, MUGGLE_AES_BLOCK_SIZE);
		}

		offset += MUGGLE_AES_BLOCK_SIZE;
	}

	return 0;
}

int muggle_aes_cfb128(
	const muggle_aes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_AES_BLOCK_SIZE],
	unsigned int *iv_offset,
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_CFB, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(iv != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(*iv_offset < MUGGLE_AES_BLOCK_SIZE, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	int op = ctx->op;
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);
	const muggle_aes_subkeys_t *sk = &ctx->sk;
	unsigned int offset = *iv_offset;

	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		if (offset == 0)
		{
			muggle_aes_crypt(MUGGLE_ENCRYPT, iv, sk, iv);
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

		offset = (offset + 1) & 0x0f;
	}

	*iv_offset = offset;

	return 0;
}

int muggle_aes_ofb128(
	const muggle_aes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char iv[MUGGLE_AES_BLOCK_SIZE],
	unsigned int *iv_offset,
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_OFB, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(iv != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(*iv_offset < MUGGLE_AES_BLOCK_SIZE, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	int op = ctx->op;
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);
	const muggle_aes_subkeys_t *sk = &ctx->sk;
	unsigned int offset = *iv_offset;

	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		if (offset == 0)
		{
			muggle_aes_crypt(MUGGLE_ENCRYPT, iv, sk, iv);
		}
		output[i] = input[i] ^ iv[offset];
		offset = (offset + 1) & 0x0f;
	}

	*iv_offset = offset;

	return 0;
}

int muggle_aes_ctr(
	const muggle_aes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	uint64_t nonce[2],
	unsigned int *nonce_offset,
	unsigned char stream_block[MUGGLE_AES_BLOCK_SIZE],
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ctx->mode == MUGGLE_BLOCK_CIPHER_MODE_CTR, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(nonce_offset != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(*nonce_offset < MUGGLE_AES_BLOCK_SIZE, MUGGLE_ERR_INVALID_PARAM);
	MUGGLE_CHECK_RET(stream_block != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(output != NULL, MUGGLE_ERR_NULL_PARAM);

	int op = ctx->op;
	MUGGLE_CHECK_RET(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, MUGGLE_ERR_INVALID_PARAM);
	const muggle_aes_subkeys_t *sk = &ctx->sk;
	unsigned int offset = *nonce_offset;

	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		if (offset == 0)
		{
			nonce[0] += 1;
			if (nonce[0] == 0)
			{
				nonce[1] += 1;
			}
			muggle_aes_crypt(MUGGLE_ENCRYPT, (unsigned char*)nonce, sk, stream_block);
		}
		output[i] = input[i] ^ stream_block[offset];
		offset = (offset + 1) & 0x0f;
	}

	*nonce_offset = offset;

	return 0;
}
