#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

#define EXAMPLE_MESSAGE_LEN 512

void gen_input_var(
	muggle_64bit_block_t *key1,
	muggle_64bit_block_t *key2,
	muggle_64bit_block_t *key3,
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

void crypt_tdes_test(int block_cipher_mode)
{
	int ret;
	muggle_64bit_block_t key1, key2, key3;
	muggle_64bit_block_t iv, iv2, iv_save;
	unsigned char input[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned int num_bytes = (unsigned int)sizeof(input);

	for (int i = 0; i < 16; ++i)
	{
		gen_input_var(&key1, &key2, &key3, &iv, input, num_bytes);
		iv_save.u64 = iv2.u64 = iv.u64;

		muggle_tdes_cipher(block_cipher_mode, MUGGLE_ENCRYPT,
			key1, key2, key3, input, num_bytes, &iv, 1, ciphertext);
		muggle_tdes_cipher(block_cipher_mode, MUGGLE_DECRYPT,
			key1, key2, key3, ciphertext, num_bytes, &iv2, 1, plaintext);

		ret = memcmp(input, plaintext, EXAMPLE_MESSAGE_LEN);
		if (ret != 0)
		{
			printf("keys1: ");
			muggle_output_hex(key1.bytes, 8, 0);
			printf("keys2: ");
			muggle_output_hex(key2.bytes, 8, 0);
			printf("keys3: ");
			muggle_output_hex(key3.bytes, 8, 0);
			printf("iv: ");
			muggle_output_hex((unsigned char*)iv_save.bytes, 8, 0);
			printf("input: \n");
			muggle_output_hex(input, num_bytes, 16);
			printf("ciphertext: \n");
			muggle_output_hex(ciphertext, num_bytes, 16);
			printf("plaintext: \n");
			muggle_output_hex(plaintext, num_bytes, 16);
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
