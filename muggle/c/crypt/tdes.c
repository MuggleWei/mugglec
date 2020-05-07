#include "tdes.h"
#include <stddef.h>
#include "muggle/c/base/utils.h"
#include "muggle/c/log/log.h"
#include "muggle/c/base/err.h"
#include "muggle/c/crypt/internal/internal_tdes.h"

fn_muggle_tdes_cipher s_fn_muggle_tdes[] = {
	muggle_tdes_ecb,
	muggle_tdes_cbc,
	muggle_tdes_cfb,
	muggle_tdes_ofb,
	muggle_tdes_ctr,
};

int muggle_tdes_crypt(
	const muggle_64bit_block_t *input,
	const muggle_des_subkeys_t *ks1,
	const muggle_des_subkeys_t *ks2,
	const muggle_des_subkeys_t *ks3,
	muggle_64bit_block_t *output)
{
	int ret = 0;
	muggle_64bit_block_t t1, t2;

	ret = muggle_des_crypt(input, ks1, &t1);
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 1");
		return ret;
	}

	ret = muggle_des_crypt(&t1, ks2, &t2);
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 2");
		return ret;
	}

	ret = muggle_des_crypt(&t2, ks3, output);
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 3");
		return ret;
	}

	return 0;
}

int muggle_tdes_cipher_bytes(
	int op,
	int block_cipher_mode,
	muggle_64bit_block_t key1,
	muggle_64bit_block_t key2,
	muggle_64bit_block_t key3,
	const unsigned char *input,
	unsigned int num_bytes,
	muggle_64bit_block_t *iv,
	int update_iv,
	unsigned char *output)
{
	muggle_des_subkeys_t ks1, ks2, ks3;
	switch (block_cipher_mode)
	{
	case MUGGLE_BLOCK_CIPHER_MODE_ECB:
	case MUGGLE_BLOCK_CIPHER_MODE_CBC:
		{
			int inv_op = op == MUGGLE_ENCRYPT ? MUGGLE_DECRYPT : MUGGLE_ENCRYPT;
			muggle_des_gen_subkeys(op, &key1, &ks1);
			muggle_des_gen_subkeys(inv_op, &key2, &ks2);
			muggle_des_gen_subkeys(op, &key3, &ks3);
		}break;
	case MUGGLE_BLOCK_CIPHER_MODE_CFB:
	case MUGGLE_BLOCK_CIPHER_MODE_OFB:
	case MUGGLE_BLOCK_CIPHER_MODE_CTR:
		{
			muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key1, &ks1);
			muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key2, &ks2);
			muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key3, &ks3);
		}break;
	default:
		{
			MUGGLE_ASSERT_MSG(block_cipher_mode >= 0 && block_cipher_mode < MAX_MUGGLE_BLOCK_CIPHER_MODE, "Invalid block cipher mode");
			return MUGGLE_ERR_INVALID_PARAM;
		};
	}

	return muggle_tdes_cipher(op, block_cipher_mode, &ks1, &ks2, &ks3, input, num_bytes, iv, update_iv, output);
}

int muggle_tdes_cipher(
	int op,
	int block_cipher_mode,
	const muggle_des_subkeys_t *ks1,
	const muggle_des_subkeys_t *ks2,
	const muggle_des_subkeys_t *ks3,
	const unsigned char *input,
	unsigned int num_bytes,
	muggle_64bit_block_t *iv,
	int update_iv,
	unsigned char *output)
{
	if (input == NULL)
	{
		MUGGLE_ASSERT_MSG(input != NULL, "TDES cipher failed: input is nullptr");
		return MUGGLE_ERR_NULL_PARAM;
	}

	if (ROUND_UP_POW_OF_2_MUL(num_bytes, 8) != num_bytes)
	{
		MUGGLE_ASSERT_MSG(ROUND_UP_POW_OF_2_MUL(num_bytes, 8) == num_bytes, "TDES cipher failed: input bytes is not multiple of 8");
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (update_iv != 0 && iv == NULL)
	{
		MUGGLE_ASSERT_MSG(!(update_iv != 0 && iv == NULL), "TDES cipher failed: iv is null and wanna update iv");
		return MUGGLE_ERR_INVALID_PARAM;
	}

	return s_fn_muggle_tdes[block_cipher_mode](op, ks1, ks2, ks3, input, num_bytes, iv, update_iv, output);
}
