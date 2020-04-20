#include "tdes.h"
#include <stddef.h>
#include "muggle/c/base/utils.h"
#include "muggle/c/log/log.h"

// extern int des_default_gen_subkey(const muggle_64bit_block_t *key, muggle_des_subkeys_t *ks);
// extern muggle_des_cb_t des_default_encrypt_callbacks; 
// extern muggle_des_cb_t des_default_decrypt_callbacks; 
// 
// 
// // callbacks
// int muggle_tdes_ecb(
// 	int enc, const unsigned char *input, unsigned char *output,	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1, muggle_des_subkeys_t *ks2, muggle_des_subkeys_t *ks3, 
// 	muggle_64bit_block_t *iv, int update_iv, muggle_des_cb_t *encrypt_cb, muggle_des_cb_t *decrypt_cb);
// int muggle_tdes_cbc(
// 	int enc, const unsigned char *input, unsigned char *output,	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1, muggle_des_subkeys_t *ks2, muggle_des_subkeys_t *ks3, 
// 	muggle_64bit_block_t *iv, int update_iv, muggle_des_cb_t *encrypt_cb, muggle_des_cb_t *decrypt_cb);
// int muggle_tdes_cfb(
// 	int enc, const unsigned char *input, unsigned char *output,	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1, muggle_des_subkeys_t *ks2, muggle_des_subkeys_t *ks3, 
// 	muggle_64bit_block_t *iv, int update_iv, muggle_des_cb_t *encrypt_cb, muggle_des_cb_t *decrypt_cb);
// int muggle_tdes_ofb(
// 	int enc, const unsigned char *input, unsigned char *output,	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1, muggle_des_subkeys_t *ks2, muggle_des_subkeys_t *ks3, 
// 	muggle_64bit_block_t *iv, int update_iv, muggle_des_cb_t *encrypt_cb, muggle_des_cb_t *decrypt_cb);
// int muggle_tdes_ctr(
// 	int enc, const unsigned char *input, unsigned char *output,	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1, muggle_des_subkeys_t *ks2, muggle_des_subkeys_t *ks3, 
// 	muggle_64bit_block_t *iv, int update_iv, muggle_des_cb_t *encrypt_cb, muggle_des_cb_t *decrypt_cb);
// 
// typedef int (*fn_muggle_tdes_cipher)(
// 	int enc,
// 	const unsigned char *input,
// 	unsigned char *output,
// 	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1,
// 	muggle_des_subkeys_t *ks2,
// 	muggle_des_subkeys_t *ks3,
// 	muggle_64bit_block_t *iv,
// 	int update_iv,
// 	muggle_des_cb_t *encrypt_cb,
// 	muggle_des_cb_t *decrypt_cb);
// 
// fn_muggle_tdes_cipher s_fn_muggle_tdes[] = {
// 	muggle_tdes_ecb,
// 	muggle_tdes_cbc,
// 	muggle_tdes_cfb,
// 	muggle_tdes_ofb,
// 	muggle_tdes_ctr,
// };
// 
// int muggle_tdes_crypt(
// 	int enc,
// 	const muggle_64bit_block_t *input,
// 	muggle_64bit_block_t *output,
// 	muggle_des_subkeys_t *ks1,
// 	muggle_des_subkeys_t *ks2,
// 	muggle_des_subkeys_t *ks3,
// 	muggle_des_cb_t *encrypt_cb,
// 	muggle_des_cb_t *decrypt_cb)
// {
// 	int ret = 0;
// 	muggle_64bit_block_t t;
// 	if (enc)
// 	{
// 		ret = muggle_des_crypt(MUGGLE_ENCRYPT, input, output, ks1, encrypt_cb);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 1 encrypt");
// 			return ret;
// 		}
// 
// 		t.u64 = output->u64;
// 		ret = muggle_des_crypt(MUGGLE_DECRYPT, &t, output, ks2, decrypt_cb);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 2 decrypt");
// 			return ret;
// 		}
// 
// 		t.u64 = output->u64;
// 		ret = muggle_des_crypt(MUGGLE_ENCRYPT, &t, output, ks3, encrypt_cb);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 3 encrypt");
// 			return ret;
// 		}
// 	}
// 	else
// 	{
// 		ret = muggle_des_crypt(MUGGLE_DECRYPT, input, output, ks3, decrypt_cb);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 1 decrypt");
// 			return ret;
// 		}
// 
// 		t.u64 = output->u64;
// 		ret = muggle_des_crypt(MUGGLE_ENCRYPT, &t, output, ks2, encrypt_cb);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 2 encrypt");
// 			return ret;
// 		}
// 
// 		t.u64 = output->u64;
// 		ret = muggle_des_crypt(MUGGLE_DECRYPT, &t, output, ks1, decrypt_cb);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES crypt failed, failed in step 3 decrypt");
// 			return ret;
// 		}
// 	}
// 
// 	return 0;
// }
// 
// int muggle_tdes_cipher(
// 	int mode,
// 	int enc,
// 	const unsigned char *input,
// 	unsigned char *output,
// 	unsigned int num_bytes,
// 	muggle_des_key_block key1,
// 	muggle_des_key_block key2,
// 	muggle_des_key_block key3,
// 	muggle_64bit_block_t *iv,
// 	int update_iv,
// 	muggle_des_cb_t *encrypt_cb,
// 	muggle_des_cb_t *decrypt_cb)
// {
// 	if (mode < 0 || mode >= MAX_MUGGLE_BLOCK_CIPHER_MODE)
// 	{
// 		MUGGLE_LOG_ERROR("invalid block cipher mode: %d", mode);
// 		return -1;
// 	}
// 
// 	if (ROUND_UP_POW_OF_2_MUL(num_bytes, 8) != num_bytes)
// 	{
// 		MUGGLE_LOG_ERROR("TDES failed: input bytes is not multiple of 8");
// 		return -1;
// 	}
// 	size_t len = num_bytes / 8;
// 
// 	// generate subkey
// 	int ret = 0;
// 	muggle_des_subkeys_t ks1, ks2, ks3;
// 	if (encrypt_cb == NULL)
// 	{
// 		ret = des_default_gen_subkey(&key1, &ks1);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES cipher failed, failed gen subkey");
// 			return -1;
// 		}
// 		ret = des_default_gen_subkey(&key2, &ks2);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES cipher failed, failed gen subkey");
// 			return -1;
// 		}
// 		ret = des_default_gen_subkey(&key3, &ks3);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES cipher failed, failed gen subkey");
// 			return -1;
// 		}
// 	}
// 	else
// 	{
// 		ret = encrypt_cb->gen_subkey(&key1, &ks1);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES cipher failed, failed gen subkey");
// 			return -1;
// 		}
// 		ret = encrypt_cb->gen_subkey(&key2, &ks2);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES cipher failed, failed gen subkey");
// 			return -1;
// 		}
// 		ret = encrypt_cb->gen_subkey(&key3, &ks3);
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES cipher failed, failed gen subkey");
// 			return -1;
// 		}
// 	}
// 
// 	if (ret != 0)
// 	{
// 		MUGGLE_LOG_ERROR("DES with CBC mode failed, failed gen subkey");
// 		return -1;
// 	}
// 
// 	return s_fn_muggle_tdes[mode](enc, input, output, num_bytes, &ks1, &ks2, &ks3, iv, update_iv, encrypt_cb, decrypt_cb);
// }
// 
// int muggle_tdes_ecb(
// 	int enc, const unsigned char *input, unsigned char *output,	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1, muggle_des_subkeys_t *ks2, muggle_des_subkeys_t *ks3, 
// 	muggle_64bit_block_t *iv, int update_iv, muggle_des_cb_t *encrypt_cb, muggle_des_cb_t *decrypt_cb)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t *input_block, *output_block;
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		ret = muggle_tdes_crypt(
// 			enc, input_block, output_block,
// 			ks1, ks2, ks3, encrypt_cb, decrypt_cb);
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
// int muggle_tdes_cbc(
// 	int enc, const unsigned char *input, unsigned char *output,	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1, muggle_des_subkeys_t *ks2, muggle_des_subkeys_t *ks3, 
// 	muggle_64bit_block_t *iv, int update_iv, muggle_des_cb_t *encrypt_cb, muggle_des_cb_t *decrypt_cb)
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
// 			ret = muggle_tdes_crypt(
// 				enc, &v, output_block,
// 				ks1, ks2, ks3, encrypt_cb, decrypt_cb);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("TDES ECB crypt failed");
// 				return ret;
// 			}
// 
// 			v.u64 = output_block->u64;
// 		}
// 		else
// 		{
// 			ret = muggle_tdes_crypt(
// 				enc, input_block, output_block,
// 				ks1, ks2, ks3, encrypt_cb, decrypt_cb);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("TDES ECB crypt failed");
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
// int muggle_tdes_cfb(
// 	int enc, const unsigned char *input, unsigned char *output,	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1, muggle_des_subkeys_t *ks2, muggle_des_subkeys_t *ks3, 
// 	muggle_64bit_block_t *iv, int update_iv, muggle_des_cb_t *encrypt_cb, muggle_des_cb_t *decrypt_cb)
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
// 			ret = muggle_tdes_crypt(
// 				enc, &v, output_block,
// 				ks1, ks2, ks3, encrypt_cb, decrypt_cb);
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
// 			ret = muggle_tdes_crypt(
// 				!enc, &v, output_block,
// 				ks1, ks2, ks3, encrypt_cb, decrypt_cb);
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
// int muggle_tdes_ofb(
// 	int enc, const unsigned char *input, unsigned char *output,	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1, muggle_des_subkeys_t *ks2, muggle_des_subkeys_t *ks3, 
// 	muggle_64bit_block_t *iv, int update_iv, muggle_des_cb_t *encrypt_cb, muggle_des_cb_t *decrypt_cb)
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
// 			ret = muggle_tdes_crypt(
// 				enc, &v, output_block,
// 				ks1, ks2, ks3, encrypt_cb, decrypt_cb);
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
// 			ret = muggle_tdes_crypt(
// 				!enc, &v, output_block,
// 				ks1, ks2, ks3, encrypt_cb, decrypt_cb);
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
// int muggle_tdes_ctr(
// 	int enc, const unsigned char *input, unsigned char *output,	unsigned int num_bytes,
// 	muggle_des_subkeys_t *ks1, muggle_des_subkeys_t *ks2, muggle_des_subkeys_t *ks3, 
// 	muggle_64bit_block_t *iv, int update_iv, muggle_des_cb_t *encrypt_cb, muggle_des_cb_t *decrypt_cb)
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
// 			ret = muggle_tdes_crypt(
// 				enc, &v, output_block,
// 				ks1, ks2, ks3, encrypt_cb, decrypt_cb);
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
// 			ret = muggle_tdes_crypt(
// 				!enc, &v, output_block,
// 				ks1, ks2, ks3, encrypt_cb, decrypt_cb);
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
