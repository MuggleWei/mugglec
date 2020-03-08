#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

void gen_input_var(
	muggle_des_key_block *key1,
	muggle_des_key_block *key2,
	muggle_des_key_block *key3,
	muggle_64bit_block_t *iv,
	unsigned char *input, unsigned int num_bytes)
{
	srand((unsigned int)time(NULL));
	key1->u32.l = (uint32_t)rand();
	key1->u32.h = (uint32_t)rand();
	key2->u32.l = (uint32_t)rand();
	key2->u32.h = (uint32_t)rand();
	key3->u32.l = (uint32_t)rand();
	key3->u32.h = (uint32_t)rand();
	iv->u32.l = (uint32_t)rand();
	iv->u32.h = (uint32_t)rand();
	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		input[i] = (unsigned char)(rand() % 256);
	}
}

void crypt_tdes_test(int mode)
{
	int ret;
	muggle_des_key_block key1, key2, key3;
	muggle_64bit_block_t iv, iv2, iv_save;
	unsigned char input[64], ciphertext[64], plaintext[64];
	for (int i = 0; i < 1024; ++i)
	{
		gen_input_var(&key1, &key2, &key3, &iv, input, sizeof(input));
		iv_save.u64 = iv2.u64 = iv.u64;

		ret = muggle_tdes_cipher(mode, MUGGLE_ENCRYPT, input, ciphertext, sizeof(input), key1, key2, key3, &iv, 1, NULL, NULL);
		ASSERT_EQ(ret, 0);

		ret = muggle_tdes_cipher(mode, MUGGLE_DECRYPT, ciphertext, plaintext, sizeof(input), key1, key2, key3, &iv2, 1, NULL, NULL);
		ASSERT_EQ(ret, 0);

		ret = memcmp(input, plaintext, sizeof(input));
		if (ret != 0)
		{
			printf("key1: ");
			muggle_output_hex((unsigned char*)key1.bytes, 8, 0);
			printf("key2: ");
			muggle_output_hex((unsigned char*)key2.bytes, 8, 0);
			printf("key3: ");
			muggle_output_hex((unsigned char*)key3.bytes, 8, 0);
			printf("iv: ");
			muggle_output_hex((unsigned char*)iv_save.bytes, 8, 0);
			printf("input: \n");
			muggle_output_hex((unsigned char*)input, 64, 8);
			printf("ciphertext: \n");
			muggle_output_hex((unsigned char*)ciphertext, 64, 8);
			printf("plaintext: \n");
			muggle_output_hex((unsigned char*)plaintext, 64, 8);
		}
		ASSERT_EQ(ret, 0);

		ASSERT_EQ(iv.u64, iv2.u64);
	}
}

TEST(crypt_tdes, ecb)
{
	crypt_tdes_test(MUGGLE_BLOCK_CIPHER_MODE_ECB);
}

TEST(crypt_tdes, cbc)
{
	crypt_tdes_test(MUGGLE_BLOCK_CIPHER_MODE_CBC);
}

TEST(crypt_tdes, cfb)
{
	crypt_tdes_test(MUGGLE_BLOCK_CIPHER_MODE_CFB);
}

TEST(crypt_tdes, ofb)
{
	crypt_tdes_test(MUGGLE_BLOCK_CIPHER_MODE_OFB);
}

TEST(crypt_tdes, ctr)
{
	crypt_tdes_test(MUGGLE_BLOCK_CIPHER_MODE_CTR);
}
