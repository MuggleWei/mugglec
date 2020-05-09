#include <stdlib.h>
#include <string.h>
#include "gtest/gtest.h"
#if MUGGLE_TEST_LINK_OPENSSL
#include "openssl/des.h"
#endif

#include "muggle/c/muggle_c.h"
#include "muggle/c/crypt/internal/internal_des.h"

void gen_input_var(muggle_64bit_block_t *key, muggle_64bit_block_t *iv, unsigned char *input, unsigned int num_bytes)
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

#define EXAMPLE_MESSAGE_LEN 64

// void crypt_des_test(int block_cipher_mode)
// {
// 	int ret;
// 	muggle_64bit_block_t key;
// 	muggle_64bit_block_t iv, iv2, iv_save;
// 	unsigned char input[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], plaintext[EXAMPLE_MESSAGE_LEN];
// 	unsigned int num_bytes = (unsigned int)sizeof(input);
// 	for (int i = 0; i < 1024; ++i)
// 	{
// 		gen_input_var(&key, &iv, input, num_bytes);
// 		iv_save.u64 = iv2.u64 = iv.u64;
// 
// 		ret = muggle_des_cipher_bytes(MUGGLE_ENCRYPT, block_cipher_mode, key, input, num_bytes, &iv, 1, ciphertext);
// 		ASSERT_EQ(ret, 0);
// 
// 		ret = muggle_des_cipher_bytes(MUGGLE_DECRYPT, block_cipher_mode, key, ciphertext, num_bytes, &iv2, 1, plaintext);
// 		ASSERT_EQ(ret, 0);
// 
// 		ret = memcmp(input, plaintext, num_bytes);
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
// 		ASSERT_EQ(iv.u64, iv2.u64);
// 
// #if MUGGLE_TEST_LINK_OPENSSL
// 		const_DES_cblock *openssl_key = (const_DES_cblock*)&key;
// 		DES_key_schedule openssl_ks;
// 		DES_set_key_unchecked(openssl_key, &openssl_ks);
// 
// 		DES_cblock openssl_iv;
// 		memcpy(openssl_iv, iv_save.bytes, 8);
// 		unsigned char openssl_output[EXAMPLE_MESSAGE_LEN];
// 
// 		int do_compare = 1;
// 		switch (block_cipher_mode)
// 		{
// 		case MUGGLE_BLOCK_CIPHER_MODE_CBC:
// 			{
// 				DES_ncbc_encrypt(input, openssl_output, num_bytes, &openssl_ks, &openssl_iv, DES_ENCRYPT);
// 			}break;
// 		case MUGGLE_BLOCK_CIPHER_MODE_CFB:
// 			{
// 				// TODO: numbit
// 				// DES_cfb_encrypt(input, openssl_output, 0, num_bytes, &openssl_ks, &openssl_iv, DES_ENCRYPT);
// 				do_compare = 0;
// 			}break;
// 		case MUGGLE_BLOCK_CIPHER_MODE_OFB:
// 			{
// 				// TODO: numbit
// 				// DES_ofb_encrypt(input, openssl_output, 0, num_bytes, &openssl_ks, &openssl_iv, DES_ENCRYPT);
// 				do_compare = 0;
// 			}break;
// 		case MUGGLE_BLOCK_CIPHER_MODE_CTR:
// 			{
// 				do_compare = 0;
// 			}break;
// 		default:
// 			{
// 				do_compare = 0;
// 			}
// 		}
// 
// 		if (do_compare == 0)
// 		{
// 			continue;
// 		}
// 
// 		ret = memcmp(ciphertext, openssl_output, num_bytes);
// 		if (ret != 0)
// 		{
// 			printf("muggle ciphertext: ");
// 			muggle_output_hex(ciphertext, EXAMPLE_MESSAGE_LEN, 16);
// 			printf("openssl ciphertext: ");
// 			muggle_output_hex(openssl_output, EXAMPLE_MESSAGE_LEN, 16);
// 		}
// 		ASSERT_EQ(ret, 0);
// 
// 		ret = memcmp(iv.bytes, openssl_iv, 8);
// 		if (ret != 0)
// 		{
// 			printf("muggle iv: ");
// 			muggle_output_hex(iv.bytes, 8, 0);
// 			printf("openssl iv: ");
// 			muggle_output_hex(openssl_iv, 8, 0);
// 		}
// 		ASSERT_EQ(ret, 0);
// #endif
// 	}
// }

TEST(crypt_des, block_size)
{
	muggle_64bit_block_t block64;
	ASSERT_EQ(sizeof(block64), 8);

	muggle_32bit_block_t block32;
	ASSERT_EQ(sizeof(block32), 4);
}

TEST(crypt_des, EncrypDecrypt)
{
	unsigned char keys[][8] = {
		{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef},
		// {0x13, 0x34, 0x57, 0x79, 0x9b, 0xbc, 0xdf, 0xf1}
	};
	unsigned char plaintexts[][8] = {
		{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xE7},
		// {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
		// {0x54, 0x68, 0x65, 0x20, 0x71, 0x75, 0x66, 0x63}
	};

	int ret = 0;
	for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
	{
		for (size_t j = 0; j < sizeof(plaintexts) / sizeof(plaintexts[0]); j++)
		{
			// gen encrypt subkey
			muggle_des_context_t ctx;
			ret = muggle_des_set_key_with_mode(
				MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, keys[i], &ctx);
			ASSERT_EQ(ret, 0);

			// encrypt
			unsigned char output[MUGGLE_DES_BLOCK_SIZE];
			ret = muggle_des_ecb(&ctx, plaintexts[j], MUGGLE_DES_BLOCK_SIZE, output);
			ASSERT_EQ(ret, 0);

			// gen decrypt subkey
			ret = muggle_des_set_key_with_mode(
				MUGGLE_DECRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, keys[i], &ctx);
			ASSERT_EQ(ret, 0);

			// decrypt
			unsigned char ret_plaintext[MUGGLE_DES_BLOCK_SIZE];
			ret = muggle_des_ecb(&ctx, output, MUGGLE_DES_BLOCK_SIZE, ret_plaintext);
			ASSERT_EQ(ret, 0);

			ret = memcmp(plaintexts[j], ret_plaintext, MUGGLE_DES_BLOCK_SIZE);
			ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
			const_DES_cblock *openssl_input = &plaintexts[j];
			const_DES_cblock *openssl_key = &keys[i];
			DES_cblock openssl_output;
			DES_key_schedule openssl_ks;

			DES_set_key_unchecked(openssl_key, &openssl_ks);
			DES_ecb_encrypt(openssl_input, &openssl_output, &openssl_ks, DES_ENCRYPT);
			ret = memcmp(output, openssl_output, 8);
			if (ret != 0)
			{
				printf("muggle ciphertext: ");
				muggle_output_hex(output, MUGGLE_DES_BLOCK_SIZE, 0);
				printf("openssl ciphertext: ");
				muggle_output_hex(openssl_output, MUGGLE_DES_BLOCK_SIZE, 0);
			}
			ASSERT_EQ(ret, 0);
#endif
		}
	}
}

// TEST(crypt_des, ecb)
// {
// 	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_ECB);
// }
// 
// TEST(crypt_des, cbc)
// {
// 	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_CBC);
// }
// 
// TEST(crypt_des, cfb)
// {
// 	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_CFB);
// }
// 
// TEST(crypt_des, ofb)
// {
// 	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_OFB);
// }
// 
// TEST(crypt_des, ctr)
// {
// 	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_CTR);
// }
