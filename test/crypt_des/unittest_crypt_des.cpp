#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

void gen_input_var(muggle_des_key_block *key, muggle_64bit_block_t *iv, unsigned char *input, unsigned int num_bytes)
{
	srand((unsigned int)time(NULL));
	key->u32.l = (uint32_t)rand();
	key->u32.h = (uint32_t)rand();
	iv->u32.l = (uint32_t)rand();
	iv->u32.h = (uint32_t)rand();
	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		input[i] = (unsigned char)(rand() % 256);
	}
}

void crypt_des_test(int mode)
{
	int ret;
	muggle_des_key_block key;
	muggle_64bit_block_t iv, iv2, iv_save;
	unsigned char input[64], ciphertext[64], plaintext[64];
	for (int i = 0; i < 1024; ++i)
	{
		gen_input_var(&key, &iv, input, sizeof(input));
		iv_save.u64 = iv2.u64 = iv.u64;

		ret = muggle_des_cipher(mode, MUGGLE_ENCRYPT, input, ciphertext, sizeof(input), key, &iv, 1, NULL);
		ASSERT_EQ(ret, 0);

		ret = muggle_des_cipher(mode, MUGGLE_DECRYPT, ciphertext, plaintext, sizeof(input), key, &iv2, 1, NULL);
		ASSERT_EQ(ret, 0);

		ret = memcmp(input, plaintext, sizeof(input));
		if (ret != 0)
		{
			printf("key: ");
			muggle_output_hex((unsigned char*)key.bytes, 8, 0);
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
