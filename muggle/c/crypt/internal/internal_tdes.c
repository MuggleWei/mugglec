#include "internal_tdes.h"
#include <stdlib.h>
#include "muggle/c/base/utils.h"
#include "muggle/c/log/log.h"
#include "muggle/c/crypt/tdes.h"

// int muggle_tdes_ecb(
// 	int op, const muggle_des_subkeys_t *ks1, const muggle_des_subkeys_t *ks2, const muggle_des_subkeys_t *ks3,
// 	const unsigned char *input, unsigned int num_bytes,
// 	muggle_64bit_block_t *iv, int update_iv, unsigned char *output)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t *input_block, *output_block;
// 
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		if (op == MUGGLE_ENCRYPT)
// 		{
// 			ret = muggle_tdes_crypt(input_block, ks1, ks2, ks3, output_block);
// 		}
// 		else
// 		{
// 			ret = muggle_tdes_crypt(input_block, ks3, ks2, ks1, output_block);
// 		}
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("TDES ECB crypt failed");
// 			return ret;
// 		}
// 		offset += 8;
// 	}
// 
// 	return 0;
// }
// 
// int muggle_tdes_cbc(
// 	int op, const muggle_des_subkeys_t *ks1, const muggle_des_subkeys_t *ks2, const muggle_des_subkeys_t *ks3,
// 	const unsigned char *input, unsigned int num_bytes,
// 	muggle_64bit_block_t *iv, int update_iv, unsigned char *output)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t v;
// 	v.u64 = iv->u64;
// 	muggle_64bit_block_t *input_block, *output_block;
// 
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		if (op == MUGGLE_ENCRYPT)
// 		{
// 			v.u64 ^= input_block->u64;
// 			ret = muggle_tdes_crypt(&v, ks1, ks2, ks3, output_block);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("TDES ECB encrypt failed");
// 				return ret;
// 			}
// 
// 			v.u64 = output_block->u64;
// 		}
// 		else
// 		{
// 			ret = muggle_tdes_crypt(input_block, ks3, ks2, ks1, output_block);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("TDES ECB decrypt failed");
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
// int muggle_tdes_cfb(
// 	int op, const muggle_des_subkeys_t *ks1, const muggle_des_subkeys_t *ks2, const muggle_des_subkeys_t *ks3,
// 	const unsigned char *input, unsigned int num_bytes,
// 	muggle_64bit_block_t *iv, int update_iv, unsigned char *output)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t v;
// 	v.u64 = iv->u64;
// 	muggle_64bit_block_t *input_block, *output_block;
// 
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		if (op == MUGGLE_ENCRYPT)
// 		{
// 			ret = muggle_tdes_crypt(&v, ks1, ks2, ks3, output_block);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("TDES CFB crypt failed");
// 				return ret;
// 			}
// 			output_block->u64 ^= input_block->u64;
// 			v.u64 = output_block->u64;
// 		}
// 		else
// 		{
// 			ret = muggle_tdes_crypt(&v, ks1, ks2, ks3, output_block);
// 			if (ret != 0)
// 			{
// 				MUGGLE_LOG_ERROR("TDES CFB crypt failed");
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
// int muggle_tdes_ofb(
// 	int op, const muggle_des_subkeys_t *ks1, const muggle_des_subkeys_t *ks2, const muggle_des_subkeys_t *ks3,
// 	const unsigned char *input, unsigned int num_bytes,
// 	muggle_64bit_block_t *iv, int update_iv, unsigned char *output)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t v;
// 	v.u64 = iv->u64;
// 	muggle_64bit_block_t *input_block, *output_block;
// 
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		if (op == MUGGLE_ENCRYPT)
// 		{
// 			ret = muggle_tdes_crypt(&v, ks1, ks2, ks3, output_block);
// 		}
// 		else
// 		{
// 			ret = muggle_tdes_crypt(&v, ks1, ks2, ks3, output_block);
// 		}
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("DES OFB crypt failed");
// 			return ret;
// 		}
// 		v.u64 = output_block->u64;
// 		output_block->u64 ^= input_block->u64;
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
// int muggle_tdes_ctr(
// 	int op, const muggle_des_subkeys_t *ks1, const muggle_des_subkeys_t *ks2, const muggle_des_subkeys_t *ks3,
// 	const unsigned char *input, unsigned int num_bytes,
// 	muggle_64bit_block_t *iv, int update_iv, unsigned char *output)
// {
// 	int ret = 0;
// 	size_t len = num_bytes / 8, offset = 0;
// 	muggle_64bit_block_t v;
// 	v.u64 = iv->u64;
// 	muggle_64bit_block_t *input_block, *output_block;
// 
// 	for (size_t i = 0; i < len; ++i)
// 	{
// 		input_block = (muggle_64bit_block_t*)(input + offset);
// 		output_block = (muggle_64bit_block_t*)(output + offset);
// 
// 		if (op == MUGGLE_ENCRYPT)
// 		{
// 			ret = muggle_tdes_crypt(&v, ks1, ks2, ks3, output_block);
// 		}
// 		else
// 		{
// 			ret = muggle_tdes_crypt(&v, ks1, ks2, ks3, output_block);
// 		}
// 		if (ret != 0)
// 		{
// 			MUGGLE_LOG_ERROR("DES CTR crypt failed");
// 			return ret;
// 		}
// 		output_block->u64 ^= input_block->u64;
// 		v.u64++;
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
