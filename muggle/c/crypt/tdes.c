#include "tdes.h"
#include <stddef.h>
#include "muggle/c/base/utils.h"
#include "muggle/c/log/log.h"
#include "muggle/c/base/err.h"
#include "muggle/c/crypt/internal/internal_des.h"
#include "muggle/c/crypt/internal/internal_tdes.h"
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
#if MUGGLE_CRYPT_USE_OPENSSL
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

	ret = muggle_des_set_key(op, mode, key1, &ctx->ctx1);
	MUGGLE_CHECK_RET(ret == 0, ret);

	ret = muggle_des_set_key(inv_op, mode, key2, &ctx->ctx2);
	MUGGLE_CHECK_RET(ret == 0, ret);

	ret = muggle_des_set_key(inv_op, mode, key3, &ctx->ctx3);
	MUGGLE_CHECK_RET(ret == 0, ret);

	return ret;
}

int muggle_tdes_ecb(
	muggle_tdes_context_t *ctx,
	const unsigned char *input,
	unsigned int num_bytes,
	unsigned char *output)
{
	MUGGLE_CHECK_RET(ctx != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(input != NULL, MUGGLE_ERR_NULL_PARAM);
	MUGGLE_CHECK_RET(ROUND_UP_POW_OF_2_MUL(num_bytes, MUGGLE_DES_BLOCK_SIZE) == num_bytes, MUGGLE_ERR_INVALID_PARAM);
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

// int muggle_tdes_crypt(
// 	const muggle_64bit_block_t *input,
// 	const muggle_des_subkeys_t *ks1,
// 	const muggle_des_subkeys_t *ks2,
// 	const muggle_des_subkeys_t *ks3,
// 	muggle_64bit_block_t *output)
// {
// 	int ret = 0;
// 	muggle_64bit_block_t t1, t2;
// 
// 	ret = muggle_des_crypt(input, ks1, &t1);
// 	if (ret != 0)
// 	{
// 		MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 1");
// 		return ret;
// 	}
// 
// 	ret = muggle_des_crypt(&t1, ks2, &t2);
// 	if (ret != 0)
// 	{
// 		MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 2");
// 		return ret;
// 	}
// 
// 	ret = muggle_des_crypt(&t2, ks3, output);
// 	if (ret != 0)
// 	{
// 		MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 3");
// 		return ret;
// 	}
// 
// 	return 0;
// }
// 
// int muggle_tdes_cipher_bytes(
// 	int op,
// 	int block_cipher_mode,
// 	muggle_64bit_block_t key1,
// 	muggle_64bit_block_t key2,
// 	muggle_64bit_block_t key3,
// 	const unsigned char *input,
// 	unsigned int num_bytes,
// 	muggle_64bit_block_t *iv,
// 	int update_iv,
// 	unsigned char *output)
// {
// 	muggle_des_subkeys_t ks1, ks2, ks3;
// 	switch (block_cipher_mode)
// 	{
// 	case MUGGLE_BLOCK_CIPHER_MODE_ECB:
// 	case MUGGLE_BLOCK_CIPHER_MODE_CBC:
// 		{
// 			int inv_op = op == MUGGLE_ENCRYPT ? MUGGLE_DECRYPT : MUGGLE_ENCRYPT;
// 			muggle_des_gen_subkeys(op, &key1, &ks1);
// 			muggle_des_gen_subkeys(inv_op, &key2, &ks2);
// 			muggle_des_gen_subkeys(op, &key3, &ks3);
// 		}break;
// 	case MUGGLE_BLOCK_CIPHER_MODE_CFB:
// 	case MUGGLE_BLOCK_CIPHER_MODE_OFB:
// 	case MUGGLE_BLOCK_CIPHER_MODE_CTR:
// 		{
// 			muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key1, &ks1);
// 			muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key2, &ks2);
// 			muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key3, &ks3);
// 		}break;
// 	default:
// 		{
// 			MUGGLE_ASSERT_MSG(block_cipher_mode >= 0 && block_cipher_mode < MAX_MUGGLE_BLOCK_CIPHER_MODE, "Invalid block cipher mode");
// 			return MUGGLE_ERR_INVALID_PARAM;
// 		};
// 	}
// 
// 	return muggle_tdes_cipher(op, block_cipher_mode, &ks1, &ks2, &ks3, input, num_bytes, iv, update_iv, output);
// }
// 
// int muggle_tdes_cipher(
// 	int op,
// 	int block_cipher_mode,
// 	const muggle_des_subkeys_t *ks1,
// 	const muggle_des_subkeys_t *ks2,
// 	const muggle_des_subkeys_t *ks3,
// 	const unsigned char *input,
// 	unsigned int num_bytes,
// 	muggle_64bit_block_t *iv,
// 	int update_iv,
// 	unsigned char *output)
// {
// 	if (input == NULL)
// 	{
// 		MUGGLE_ASSERT_MSG(input != NULL, "TDES cipher failed: input is nullptr");
// 		return MUGGLE_ERR_NULL_PARAM;
// 	}
// 
// 	if (ROUND_UP_POW_OF_2_MUL(num_bytes, 8) != num_bytes)
// 	{
// 		MUGGLE_ASSERT_MSG(ROUND_UP_POW_OF_2_MUL(num_bytes, 8) == num_bytes, "TDES cipher failed: input bytes is not multiple of 8");
// 		return MUGGLE_ERR_INVALID_PARAM;
// 	}
// 
// 	if (update_iv != 0 && iv == NULL)
// 	{
// 		MUGGLE_ASSERT_MSG(!(update_iv != 0 && iv == NULL), "TDES cipher failed: iv is null and wanna update iv");
// 		return MUGGLE_ERR_INVALID_PARAM;
// 	}
// 
// 	return s_fn_muggle_tdes[block_cipher_mode](op, ks1, ks2, ks3, input, num_bytes, iv, update_iv, output);
// }
