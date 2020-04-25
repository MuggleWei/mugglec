#include "tdes.h"
#include <stddef.h>
#include "muggle/c/base/utils.h"
#include "muggle/c/log/log.h"
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
	muggle_des_subkeys_t *ks1,
	muggle_des_subkeys_t *ks2,
	muggle_des_subkeys_t *ks3,
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
	muggle_output_hex(t1.bytes, 8, 0);

	ret = muggle_des_crypt(&t1, ks2, &t2);
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 2");
		return ret;
	}
	muggle_output_hex(t2.bytes, 8, 0);

	ret = muggle_des_crypt(&t2, ks3, output);
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 3");
		return ret;
	}
	muggle_output_hex(output->bytes, 8, 0);

	return 0;
}

int muggle_tdes_cipher(
	int block_cipher_mode,
	int op,
	muggle_64bit_block_t key1,
	muggle_64bit_block_t key2,
	muggle_64bit_block_t key3,
	const unsigned char *input,
	unsigned int num_bytes,
	muggle_64bit_block_t *iv,
	int update_iv,
	unsigned char *output)
{
	if (block_cipher_mode < 0 || block_cipher_mode >= MAX_MUGGLE_BLOCK_CIPHER_MODE)
	{
		MUGGLE_LOG_ERROR("TDES cipher failed: invalid block cipher mode: %d", block_cipher_mode);
		return -1;
	}

	if (input == NULL)
	{
		MUGGLE_LOG_ERROR("TDES cipher failed: input is nullptr");
		return -1;
	}

	if (ROUND_UP_POW_OF_2_MUL(num_bytes, 8) != num_bytes)
	{
		MUGGLE_LOG_ERROR("TDES failed: input bytes is not multiple of 8");
		return -1;
	}

	return s_fn_muggle_tdes[block_cipher_mode](op, key1, key2, key3, input, num_bytes, iv, update_iv, output);
}
