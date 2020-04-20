#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

// void gen_input_var(muggle_des_key_block *key, muggle_64bit_block_t *iv, unsigned char *input, unsigned int num_bytes)
// {
// 	srand((unsigned int)time(NULL));
// 	key->u32.l = (uint32_t)rand();
// 	key->u32.h = (uint32_t)rand();
// 	iv->u32.l = (uint32_t)rand();
// 	iv->u32.h = (uint32_t)rand();
// 	for (unsigned int i = 0; i < num_bytes; ++i)
// 	{
// 		input[i] = (unsigned char)(rand() % 256);
// 	}
// }
// 
// void crypt_des_test(int mode)
// {
// 	int ret;
// 	muggle_des_key_block key;
// 	muggle_64bit_block_t iv, iv2, iv_save;
// 	unsigned char input[64], ciphertext[64], plaintext[64];
// 	for (int i = 0; i < 1024; ++i)
// 	{
// 		gen_input_var(&key, &iv, input, sizeof(input));
// 		iv_save.u64 = iv2.u64 = iv.u64;
// 
// 		ret = muggle_des_cipher(mode, MUGGLE_ENCRYPT, input, ciphertext, sizeof(input), key, &iv, 1, NULL);
// 		ASSERT_EQ(ret, 0);
// 
// 		ret = muggle_des_cipher(mode, MUGGLE_DECRYPT, ciphertext, plaintext, sizeof(input), key, &iv2, 1, NULL);
// 		ASSERT_EQ(ret, 0);
// 
// 		ret = memcmp(input, plaintext, sizeof(input));
// 		if (ret != 0)
// 		{
// 			printf("key: ");
// 			muggle_output_hex((unsigned char*)key.bytes, 8, 0);
// 			printf("iv: ");
// 			muggle_output_hex((unsigned char*)iv_save.bytes, 8, 0);
// 			printf("input: \n");
// 			muggle_output_hex((unsigned char*)input, 64, 8);
// 			printf("ciphertext: \n");
// 			muggle_output_hex((unsigned char*)ciphertext, 64, 8);
// 			printf("plaintext: \n");
// 			muggle_output_hex((unsigned char*)plaintext, 64, 8);
// 		}
// 		ASSERT_EQ(ret, 0);
// 
// 		ASSERT_EQ(iv.u64, iv2.u64);
// 	}
// }

TEST(crypt_des, block_size)
{
	muggle_64bit_block_t block;
	ASSERT_EQ(sizeof(block), 8);
}

void test_crypt_des_ip_bit_perm(unsigned int from, unsigned int to)
{
	muggle_64bit_block_t in;
	muggle_64bit_block_t out;

	// the bit need offset
	memset(&in, 0, sizeof(in));
	in.u64 = 0x01LLU << from;
	muggle_des_ip(&in, &out);
	ASSERT_EQ(out.u64, 0x01LLU << to);
}

void test_crypt_des_fp_bit_perm(unsigned int from, unsigned int to)
{
	muggle_64bit_block_t in;
	muggle_64bit_block_t out;

	// the bit need offset
	memset(&in, 0, sizeof(in));
	in.u64 = 0x01LLU << from;
	muggle_des_fp(&in, &out);
	ASSERT_EQ(out.u64, 0x01LLU << to);
}

TEST(crypt_des, IP)
{
	uint32_t ip_table[] = {
		58, 50, 42, 34, 26, 18, 10, 2,
		60, 52, 44, 36, 28, 20, 12, 4,
		62, 54, 46, 38, 30, 22, 14, 6,
		64, 56, 48, 40, 32, 24, 16, 8,
		57, 49, 41, 33, 25, 17, 9,  1,
		59, 51, 43, 35, 27, 19, 11, 3,
		61, 53, 45, 37, 29, 21, 13, 5,
		63, 55, 47, 39, 31, 23, 15, 7
	};
	for (uint32_t i = 0; i < sizeof(ip_table) / sizeof(ip_table[0]); i++)
	{
		test_crypt_des_ip_bit_perm(ip_table[i]-1, i);
	}
}

TEST(crypt_des, FP)
{
	uint32_t fp_table[] = {
		40, 8, 48, 16, 56, 24, 64, 32,
		39, 7, 47, 15, 55, 23, 63, 31,
		38, 6, 46, 14, 54, 22, 62, 30,
		37, 5, 45, 13, 53, 21, 61, 29,
		36, 4, 44, 12, 52, 20, 60, 28,
		35, 3, 43, 11, 51, 19, 59, 27,
		34, 2, 42, 10, 50, 18, 58, 26,
		33, 1, 41, 9,  49, 17, 57, 25
	};
	for (uint32_t i = 0; i < sizeof(fp_table) / sizeof(fp_table[0]); i++)
	{
		test_crypt_des_fp_bit_perm(fp_table[i]-1, i);
	}
}

TEST(crypt_des, IPFP)
{
	uint32_t ip_table[] = {
		58, 50, 42, 34, 26, 18, 10, 2,
		60, 52, 44, 36, 28, 20, 12, 4,
		62, 54, 46, 38, 30, 22, 14, 6,
		64, 56, 48, 40, 32, 24, 16, 8,
		57, 49, 41, 33, 25, 17, 9,  1,
		59, 51, 43, 35, 27, 19, 11, 3,
		61, 53, 45, 37, 29, 21, 13, 5,
		63, 55, 47, 39, 31, 23, 15, 7
	};

	muggle_64bit_block_t in, out1, out2;

	// the bit need offset
	for (uint32_t i = 0; i < sizeof(ip_table) / sizeof(ip_table[0]); i++)
	{
		uint32_t from = ip_table[i] - 1;
		memset(&in, 0, sizeof(in));
		in.u64 = 0x01LLU << from;
		muggle_des_ip(&in, &out1);
		muggle_des_fp(&out1, &out2);
		ASSERT_EQ(in.u64, out2.u64);
	}
}

/*
TEST(crypt_des, ecb)
{
	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_ECB);
}

TEST(crypt_des, cbc)
{
	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_CBC);
}

TEST(crypt_des, cfb)
{
	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_CFB);
}

TEST(crypt_des, ofb)
{
	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_OFB);
}

TEST(crypt_des, ctr)
{
	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_CTR);
}
*/
