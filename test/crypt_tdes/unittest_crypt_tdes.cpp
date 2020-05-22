#include <stdlib.h>
#include <string.h>
#include "gtest/gtest.h"
#if MUGGLE_TEST_LINK_OPENSSL
#include "openssl/des.h"
#endif
#include "muggle/c/muggle_c.h"

#define TEST_SPACE_SIZE 512

void gen_input_var(
	unsigned char key1[MUGGLE_DES_BLOCK_SIZE],
	unsigned char key2[MUGGLE_DES_BLOCK_SIZE],
	unsigned char key3[MUGGLE_DES_BLOCK_SIZE],
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE],
	unsigned char *input,
	unsigned int num_bytes)
{
	srand((unsigned int)time(NULL));
	uint32_t r = 0;

	r = (uint32_t)rand();
	memcpy(&key1[0], &r, 4);
	r = (uint32_t)rand();
	memcpy(&key1[4], &r, 4);

	r = (uint32_t)rand();
	memcpy(&key2[0], &r, 4);
	r = (uint32_t)rand();
	memcpy(&key2[4], &r, 4);

	r = (uint32_t)rand();
	memcpy(&key3[0], &r, 4);
	r = (uint32_t)rand();
	memcpy(&key3[4], &r, 4);

	r = (uint32_t)rand();
	memcpy(&iv[0], &r, 4);
	r = (uint32_t)rand();
	memcpy(&iv[4], &r, 4);
	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		input[i] = (unsigned char)(rand() % 256);
	}
}

TEST(crypt_tdes, EncrypDecrypt)
{
 	int ret;
 	unsigned char key1[MUGGLE_DES_BLOCK_SIZE], key2[MUGGLE_DES_BLOCK_SIZE], key3[MUGGLE_DES_BLOCK_SIZE];
 	unsigned char plaintext[MUGGLE_DES_BLOCK_SIZE], ciphertext[MUGGLE_DES_BLOCK_SIZE], ret_plaintext[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE];
 	unsigned int num_bytes = (unsigned int)sizeof(plaintext);
	for (int i = 0; i < 1; i++)
	{
		memset(plaintext, 0, sizeof(plaintext));
		memset(ciphertext, 0, sizeof(ciphertext));

		gen_input_var(key1, key2, key3, iv, plaintext, num_bytes);

		// gen encrypt subkey
		muggle_tdes_context_t ctx;
		ret = muggle_tdes_set_key(
			MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, key1, key2, key3, &ctx);
		ASSERT_EQ(ret, 0);

		// encrypt
		ret = muggle_tdes_ecb(&ctx, plaintext, num_bytes, ciphertext);
		ASSERT_EQ(ret, 0);

		// gen decrypt subkey
		ret = muggle_tdes_set_key(
			MUGGLE_DECRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, key1, key2, key3, &ctx);
		ASSERT_EQ(ret, 0);

		// decrypt
		ret = muggle_tdes_ecb(&ctx, ciphertext, num_bytes, ret_plaintext);
		ASSERT_EQ(ret, 0);

		ret = memcmp(plaintext, ret_plaintext, num_bytes);
		ASSERT_EQ(ret, 0);

 #if MUGGLE_TEST_LINK_OPENSSL
 		const_DES_cblock *openssl_input = &plaintext;
 		const_DES_cblock *openssl_key1 = &key1;
 		const_DES_cblock *openssl_key2 = &key2;
 		const_DES_cblock *openssl_key3 = &key3;
 		DES_cblock openssl_output;
 		DES_key_schedule openssl_ks1, openssl_ks2, openssl_ks3;
 
 		DES_set_key_unchecked(openssl_key1, &openssl_ks1);
 		DES_set_key_unchecked(openssl_key2, &openssl_ks2);
 		DES_set_key_unchecked(openssl_key3, &openssl_ks3);
 		DES_ecb3_encrypt(
 			openssl_input, &openssl_output, 
 			&openssl_ks1, &openssl_ks2, &openssl_ks3, DES_ENCRYPT);
 		ret = memcmp(ciphertext, openssl_output, 8);
 		if (ret != 0)
 		{
 			printf("muggle ciphertext: ");
 			muggle_output_hex(ciphertext, MUGGLE_DES_BLOCK_SIZE, 0);
 			printf("openssl ciphertext: ");
 			muggle_output_hex(openssl_output, MUGGLE_DES_BLOCK_SIZE, 0);
 		}
 		ASSERT_EQ(ret, 0);
 #endif
	}
}

TEST(crypt_tdes, ecb)
{
	int ret;
	unsigned char key1[MUGGLE_DES_BLOCK_SIZE], key2[MUGGLE_DES_BLOCK_SIZE], key3[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE];
	unsigned char plaintext[TEST_SPACE_SIZE], ciphertext[TEST_SPACE_SIZE], ret_plaintext[TEST_SPACE_SIZE];
	unsigned int num_bytes = TEST_SPACE_SIZE;
	for (int i = 0; i < 16; ++i)
	{
		memset(plaintext, 0, sizeof(plaintext));
		memset(ciphertext, 0, sizeof(ciphertext));
		memset(ret_plaintext, 0, sizeof(ret_plaintext));

		gen_input_var(key1, key2, key3, iv, plaintext, num_bytes);

		// gen encrypt subkey
		muggle_tdes_context_t ctx;
		ret = muggle_tdes_set_key(
			MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, 
			key1, key2, key3, &ctx);
		ASSERT_EQ(ret, 0);

		// encrypt
		ret = muggle_tdes_ecb(&ctx, plaintext, num_bytes, ciphertext);
		ASSERT_EQ(ret, 0);

		// gen decrypt subkey
		ret = muggle_tdes_set_key(
			MUGGLE_DECRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB,
			key1, key2, key3, &ctx);
		ASSERT_EQ(ret, 0);

		// decrypt
		ret = muggle_tdes_ecb(&ctx, ciphertext, num_bytes, ret_plaintext);
		ASSERT_EQ(ret, 0);

		ret = memcmp(plaintext, ret_plaintext, num_bytes);
		ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
		unsigned char openssl_ciphertext[TEST_SPACE_SIZE];

		DES_key_schedule openssl_ks1, openssl_ks2, openssl_ks3;
		const_DES_cblock *openssl_key1 = (const_DES_cblock*)&key1;
		const_DES_cblock *openssl_key2 = (const_DES_cblock*)&key2;
		const_DES_cblock *openssl_key3 = (const_DES_cblock*)&key3;
		DES_set_key_unchecked(openssl_key1, &openssl_ks1);
		DES_set_key_unchecked(openssl_key2, &openssl_ks2);
		DES_set_key_unchecked(openssl_key3, &openssl_ks3);

		unsigned char *in = NULL, *out = NULL;
		unsigned int block_cnt = num_bytes / MUGGLE_DES_BLOCK_SIZE;
		for (unsigned int i = 0; i < block_cnt; i++)
		{
			in = (unsigned char*)plaintext + MUGGLE_DES_BLOCK_SIZE * i;
			const_DES_cblock *openssl_input = (const_DES_cblock*)in;

			out = (unsigned char*)openssl_ciphertext + MUGGLE_DES_BLOCK_SIZE * i;
			DES_cblock *openssl_output = (DES_cblock*)out;

			DES_ecb3_encrypt(openssl_input, openssl_output,
				&openssl_ks1, &openssl_ks2, &openssl_ks3, DES_ENCRYPT);
		}

		ret = memcmp(ciphertext, openssl_ciphertext, num_bytes);
		if (ret != 0)
		{
			printf("muggle ciphertext: ");
			muggle_output_hex(ciphertext, num_bytes, 32);
			printf("openssl ciphertext: ");
			muggle_output_hex(openssl_ciphertext, num_bytes, 32);
		}
		ASSERT_EQ(ret, 0);
#endif
	}
}

TEST(crypt_tdes, cbc)
{
	int ret;
	unsigned char key1[MUGGLE_DES_BLOCK_SIZE], key2[MUGGLE_DES_BLOCK_SIZE], key3[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE], iv2[MUGGLE_DES_BLOCK_SIZE], iv3[MUGGLE_DES_BLOCK_SIZE];
	unsigned char plaintext[TEST_SPACE_SIZE], ciphertext[TEST_SPACE_SIZE], ret_plaintext[TEST_SPACE_SIZE];
	unsigned int num_bytes = TEST_SPACE_SIZE;

	for (int i = 0; i < 16; ++i)
	{
		memset(plaintext, 0, sizeof(plaintext));
		memset(ciphertext, 0, sizeof(ciphertext));
		memset(ret_plaintext, 0, sizeof(ret_plaintext));

		gen_input_var(key1, key2, key3, iv, plaintext, num_bytes);
		memcpy(iv2, iv, MUGGLE_DES_BLOCK_SIZE);
		memcpy(iv3, iv, MUGGLE_DES_BLOCK_SIZE);

		// gen encrypt subkey
		muggle_tdes_context_t ctx;
		ret = muggle_tdes_set_key(
			MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_CBC,
			key1, key2, key3, &ctx);
		ASSERT_EQ(ret, 0);

		// encrypt
		ret = muggle_tdes_cbc(&ctx, plaintext, num_bytes, iv, ciphertext);
		ASSERT_EQ(ret, 0);

		// gen decrypt subkey
		ret = muggle_tdes_set_key(
			MUGGLE_DECRYPT, MUGGLE_BLOCK_CIPHER_MODE_CBC,
			key1, key2, key3, &ctx);
		ASSERT_EQ(ret, 0);

		// decrypt
		ret = muggle_tdes_cbc(&ctx, ciphertext, num_bytes, iv2, ret_plaintext);
		ASSERT_EQ(ret, 0);

		ret = memcmp(plaintext, ret_plaintext, num_bytes);
		ASSERT_EQ(ret, 0);

		ret = memcmp(iv, iv2, MUGGLE_DES_BLOCK_SIZE);
		ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
		unsigned char openssl_ciphertext[TEST_SPACE_SIZE];

		DES_key_schedule openssl_ks1, openssl_ks2, openssl_ks3;
		const_DES_cblock *openssl_key1 = (const_DES_cblock*)&key1;
		const_DES_cblock *openssl_key2 = (const_DES_cblock*)&key2;
		const_DES_cblock *openssl_key3 = (const_DES_cblock*)&key3;
		DES_set_key_unchecked(openssl_key1, &openssl_ks1);
		DES_set_key_unchecked(openssl_key2, &openssl_ks2);
		DES_set_key_unchecked(openssl_key3, &openssl_ks3);

		DES_ede3_cbc_encrypt(
			plaintext, openssl_ciphertext, (long)num_bytes,
			&openssl_ks1, &openssl_ks2, &openssl_ks3, (DES_cblock*)iv3, 1);

		ret = memcmp(ciphertext, openssl_ciphertext, num_bytes);
		if (ret != 0)
		{
			printf("muggle ciphertext:\n");
			muggle_output_hex(ciphertext, num_bytes, 32);
			printf("openssl ciphertext:\n");
			muggle_output_hex(openssl_ciphertext, num_bytes, 32);
		}
		ASSERT_EQ(ret, 0);

		ret = memcmp(iv, iv3, MUGGLE_DES_BLOCK_SIZE);
		ASSERT_EQ(ret, 0);
#endif
	}
}

TEST(crypt_tdes, cfb64)
{
	int ret;
	unsigned char key1[MUGGLE_DES_BLOCK_SIZE], key2[MUGGLE_DES_BLOCK_SIZE], key3[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE], iv2[MUGGLE_DES_BLOCK_SIZE], iv3[MUGGLE_DES_BLOCK_SIZE];
	unsigned int iv_off = 0, iv_off2 = 0;
	unsigned char plaintext_arr[TEST_SPACE_SIZE], ciphertext_arr[TEST_SPACE_SIZE], ret_plaintext_arr[TEST_SPACE_SIZE];
	unsigned char *plaintext, *ciphertext, *ret_plaintext;
	muggle_tdes_context_t encrypt_ctx, decrypt_ctx;
	unsigned int num_bytes =0, remain_bytes = 0;
	int iv_off3 = 0;
	unsigned int offset = 0;
#if MUGGLE_TEST_LINK_OPENSSL
	unsigned char openssl_ciphertext_arr[TEST_SPACE_SIZE];
	unsigned char *openssl_ciphertext;
	DES_key_schedule openssl_ks1, openssl_ks2, openssl_ks3;
	const_DES_cblock *openssl_key1 = (const_DES_cblock*)&key1;
	const_DES_cblock *openssl_key2 = (const_DES_cblock*)&key2;
	const_DES_cblock *openssl_key3 = (const_DES_cblock*)&key3;
#endif

	for (int i = 0; i < 16; ++i)
	{
		memset(plaintext_arr, 0, sizeof(plaintext_arr));
		memset(ciphertext_arr, 0, sizeof(plaintext_arr));

		gen_input_var(key1, key2, key3, iv, plaintext_arr, TEST_SPACE_SIZE);
		memcpy(iv2, iv, MUGGLE_DES_BLOCK_SIZE);
		memcpy(iv3, iv, MUGGLE_DES_BLOCK_SIZE);
		iv_off = iv_off2 = 0;
		iv_off3 = 0;
		offset = 0;

		// gen encrypt subkey
		ret = muggle_tdes_set_key(
			MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_CFB,
			key1, key2, key3, &encrypt_ctx);
		ASSERT_EQ(ret, 0);

		// gen decrypt subkey
		ret = muggle_tdes_set_key(
			MUGGLE_DECRYPT, MUGGLE_BLOCK_CIPHER_MODE_CFB,
			key1, key2, key3, &decrypt_ctx);
		ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
		DES_set_key_unchecked(openssl_key1, &openssl_ks1);
		DES_set_key_unchecked(openssl_key2, &openssl_ks2);
		DES_set_key_unchecked(openssl_key3, &openssl_ks3);
#endif

		remain_bytes = TEST_SPACE_SIZE;
		while (remain_bytes > 0)
		{
			num_bytes = rand() % 16 + 1;
			num_bytes = num_bytes > remain_bytes ? remain_bytes : num_bytes;
			remain_bytes -= num_bytes;

			plaintext = (unsigned char*)plaintext_arr + offset;
			ciphertext = (unsigned char*)ciphertext_arr + offset;
			ret_plaintext = (unsigned char*)ret_plaintext_arr + offset;

			// encrypt
			ret = muggle_tdes_cfb64(&encrypt_ctx, plaintext, num_bytes, iv, &iv_off, ciphertext);
			ASSERT_EQ(ret, 0);

			// decrypt
			ret = muggle_tdes_cfb64(&decrypt_ctx, ciphertext, num_bytes, iv2, &iv_off2, ret_plaintext);
			ASSERT_EQ(ret, 0);

			ret = memcmp(plaintext, ret_plaintext, num_bytes);
			ASSERT_EQ(ret, 0);

			ret = memcmp(iv, iv2, MUGGLE_DES_BLOCK_SIZE);
			ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
			openssl_ciphertext = (unsigned char*)openssl_ciphertext_arr + offset;
			DES_ede3_cfb64_encrypt(
					plaintext, openssl_ciphertext, (long)num_bytes,
					&openssl_ks1, &openssl_ks2, &openssl_ks3,
					(DES_cblock*)iv3, &iv_off3, 1);

			ret = memcmp(ciphertext, openssl_ciphertext, num_bytes);
			if (ret != 0)
			{
				printf("muggle ciphertext:\n");
				muggle_output_hex(ciphertext, num_bytes, 32);
				printf("openssl ciphertext:\n");
				muggle_output_hex(openssl_ciphertext, num_bytes, 32);
			}
			ASSERT_EQ(ret, 0);

			ret = memcmp(iv, iv3, MUGGLE_DES_BLOCK_SIZE);
			ASSERT_EQ(ret, 0);
#endif

			offset += num_bytes;
		}
	}
}

// TEST(crypt_tdes, ofb)
// {
// 	crypt_tdes_test(MUGGLE_BLOCK_CIPHER_MODE_OFB);
// }
// 
// TEST(crypt_tdes, ctr)
// {
// 	crypt_tdes_test(MUGGLE_BLOCK_CIPHER_MODE_CTR);
// }
