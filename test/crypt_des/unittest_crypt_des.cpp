#include <stdlib.h>
#include <string.h>
#include "gtest/gtest.h"
#if MUGGLE_TEST_LINK_OPENSSL
#include "openssl/des.h"
#endif
#include "muggle/c/muggle_c.h"

#define TEST_SPACE_SIZE 512

void gen_input_var(
	unsigned char key[MUGGLE_DES_BLOCK_SIZE],
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE],
	unsigned char *input,
	unsigned int num_bytes)
{
	srand((unsigned int)time(NULL));
	uint32_t r = 0;

	r = (uint32_t)rand();
	memcpy(&key[0], &r, 4);
	r = (uint32_t)rand();
	memcpy(&key[4], &r, 4);

	r = (uint32_t)rand();
	memcpy(&iv[0], &r, 4);
	r = (uint32_t)rand();
	memcpy(&iv[4], &r, 4);
	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		input[i] = (unsigned char)(rand() % 256);
	}
}

TEST(crypt_des, block_size)
{
	ASSERT_EQ(sizeof(muggle_64bit_block_t), (size_t)8);
	ASSERT_EQ(sizeof(muggle_32bit_block_t), (size_t)4);
}

TEST(crypt_des, EncrypDecrypt)
{
	unsigned char keys[][8] = {
		{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef},
		{0x13, 0x34, 0x57, 0x79, 0x9b, 0xbc, 0xdf, 0xf1}
	};
	unsigned char plaintexts[][8] = {
		{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xE7},
		{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
		{0x54, 0x68, 0x65, 0x20, 0x71, 0x75, 0x66, 0x63}
	};

	int ret = 0;
	for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
	{
		for (size_t j = 0; j < sizeof(plaintexts) / sizeof(plaintexts[0]); j++)
		{
			// gen encrypt subkey
			muggle_des_context_t ctx;
			ret = muggle_des_set_key(
				MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, keys[i], &ctx);
			ASSERT_EQ(ret, 0);

			// encrypt
			unsigned char output[MUGGLE_DES_BLOCK_SIZE];
			ret = muggle_des_ecb(&ctx, plaintexts[j], MUGGLE_DES_BLOCK_SIZE, output);
			ASSERT_EQ(ret, 0);

			// gen decrypt subkey
			ret = muggle_des_set_key(
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
			ret = memcmp(output, openssl_output, MUGGLE_DES_BLOCK_SIZE);
			if (ret != 0)
			{
				printf("muggle ciphertext: ");
				muggle_output_hex(output, MUGGLE_DES_BLOCK_SIZE, 8);
				printf("openssl ciphertext: ");
				muggle_output_hex(openssl_output, MUGGLE_DES_BLOCK_SIZE, 8);
			}
			ASSERT_EQ(ret, 0);
#endif
		}
	}
}

TEST(crypt_des, ecb)
{
	int ret;
	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE];
	unsigned char plaintext[TEST_SPACE_SIZE], ciphertext[TEST_SPACE_SIZE], ret_plaintext[TEST_SPACE_SIZE];
	unsigned int num_bytes = TEST_SPACE_SIZE;
	for (int i = 0; i < 16; ++i)
	{
		memset(plaintext, 0, sizeof(plaintext));
		memset(ciphertext, 0, sizeof(ciphertext));
		memset(ret_plaintext, 0, sizeof(ret_plaintext));

		gen_input_var(key, iv, plaintext, num_bytes);

		// gen encrypt subkey
		muggle_des_context_t ctx;
		ret = muggle_des_set_key(
			MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, key, &ctx);
		ASSERT_EQ(ret, 0);

		// encrypt
		ret = muggle_des_ecb(&ctx, plaintext, num_bytes, ciphertext);
		ASSERT_EQ(ret, 0);

		// gen decrypt subkey
		ret = muggle_des_set_key(
			MUGGLE_DECRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, key, &ctx);
		ASSERT_EQ(ret, 0);

		// decrypt
		ret = muggle_des_ecb(&ctx, ciphertext, num_bytes, ret_plaintext);
		ASSERT_EQ(ret, 0);

		ret = memcmp(plaintext, ret_plaintext, num_bytes);
		ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
		unsigned char openssl_ciphertext[TEST_SPACE_SIZE];

		DES_key_schedule openssl_ks;
		const_DES_cblock *openssl_key = (const_DES_cblock*)&key;
		DES_set_key_unchecked(openssl_key, &openssl_ks);

		unsigned char *in = NULL, *out = NULL;
		unsigned int block_cnt = num_bytes / MUGGLE_DES_BLOCK_SIZE;
		for (unsigned int i = 0; i < block_cnt; i++)
		{
			in = (unsigned char*)plaintext + MUGGLE_DES_BLOCK_SIZE * i;
			const_DES_cblock *openssl_input = (const_DES_cblock*)in;

			out = (unsigned char*)openssl_ciphertext + MUGGLE_DES_BLOCK_SIZE * i;
			DES_cblock *openssl_output = (DES_cblock*)out;

			DES_ecb_encrypt(openssl_input, openssl_output, &openssl_ks, DES_ENCRYPT);
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

TEST(crypt_des, cbc)
{
	int ret;
	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE], iv2[MUGGLE_DES_BLOCK_SIZE], iv3[MUGGLE_DES_BLOCK_SIZE];
	unsigned char plaintext[TEST_SPACE_SIZE], ciphertext[TEST_SPACE_SIZE], ret_plaintext[TEST_SPACE_SIZE];
	unsigned int num_bytes = TEST_SPACE_SIZE;

	for (int i = 0; i < 16; ++i)
	{
		memset(plaintext, 0, sizeof(plaintext));
		memset(ciphertext, 0, sizeof(ciphertext));
		memset(ret_plaintext, 0, sizeof(ret_plaintext));

		gen_input_var(key, iv, plaintext, num_bytes);
		memcpy(iv2, iv, MUGGLE_DES_BLOCK_SIZE);
		memcpy(iv3, iv, MUGGLE_DES_BLOCK_SIZE);

		// gen encrypt subkey
		muggle_des_context_t ctx;
		ret = muggle_des_set_key(
			MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_CBC, key, &ctx);
		ASSERT_EQ(ret, 0);

		// encrypt
		ret = muggle_des_cbc(&ctx, plaintext, num_bytes, iv, ciphertext);
		ASSERT_EQ(ret, 0);

		// gen decrypt subkey
		ret = muggle_des_set_key(
			MUGGLE_DECRYPT, MUGGLE_BLOCK_CIPHER_MODE_CBC, key, &ctx);
		ASSERT_EQ(ret, 0);

		// decrypt
		ret = muggle_des_cbc(&ctx, ciphertext, num_bytes, iv2, ret_plaintext);
		ASSERT_EQ(ret, 0);

		ret = memcmp(plaintext, ret_plaintext, num_bytes);
		ASSERT_EQ(ret, 0);

		ret = memcmp(iv, iv2, MUGGLE_DES_BLOCK_SIZE);
		ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
		unsigned char openssl_ciphertext[TEST_SPACE_SIZE];

		DES_key_schedule openssl_ks;
		const_DES_cblock *openssl_key = (const_DES_cblock*)&key;
		DES_set_key_unchecked(openssl_key, &openssl_ks);

		DES_ncbc_encrypt(plaintext, openssl_ciphertext, (long)num_bytes, &openssl_ks, (DES_cblock*)iv3, 1);

		ret = memcmp(ciphertext, openssl_ciphertext, num_bytes);
		if (ret != 0)
		{
			printf("muggle ciphertext: ");
			muggle_output_hex(ciphertext, num_bytes, 32);
			printf("openssl ciphertext: ");
			muggle_output_hex(openssl_ciphertext, num_bytes, 32);
		}
		ASSERT_EQ(ret, 0);

		ret = memcmp(iv, iv3, MUGGLE_DES_BLOCK_SIZE);
		ASSERT_EQ(ret, 0);
#endif
	}
}

TEST(crypt_des, cfb64)
{
	int ret;
	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE], iv2[MUGGLE_DES_BLOCK_SIZE], iv3[MUGGLE_DES_BLOCK_SIZE];
	unsigned int iv_off = 0, iv_off2 = 0;
	unsigned char plaintext_arr[TEST_SPACE_SIZE], ciphertext_arr[TEST_SPACE_SIZE], ret_plaintext_arr[TEST_SPACE_SIZE];
	unsigned char *plaintext, *ciphertext, *ret_plaintext;
	muggle_des_context_t encrypt_ctx, decrypt_ctx;
	unsigned int num_bytes = 0, remain_bytes = 0;
	int iv_off3 = 0;
	unsigned int offset = 0;
#if MUGGLE_TEST_LINK_OPENSSL
	unsigned char openssl_ciphertext_arr[TEST_SPACE_SIZE];
	unsigned char *openssl_ciphertext;
	DES_key_schedule openssl_ks;
#endif

	for (int i = 0; i < 16; ++i)
	{
		memset(plaintext_arr, 0, sizeof(plaintext_arr));
		memset(ciphertext_arr, 0, sizeof(ciphertext_arr));

		gen_input_var(key, iv, plaintext_arr, TEST_SPACE_SIZE);
		memcpy(iv2, iv, MUGGLE_DES_BLOCK_SIZE);
		memcpy(iv3, iv, MUGGLE_DES_BLOCK_SIZE);
		iv_off = iv_off2 = 0;
		iv_off3 = 0;
		offset = 0;

		// gen encrypt subkey
		ret = muggle_des_set_key(
			MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_CFB, key, &encrypt_ctx);
		ASSERT_EQ(ret, 0);

		// gen decrypt subkey
		ret = muggle_des_set_key(
			MUGGLE_DECRYPT, MUGGLE_BLOCK_CIPHER_MODE_CFB, key, &decrypt_ctx);
		ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
		const_DES_cblock *openssl_key = (const_DES_cblock*)&key;
		DES_set_key_unchecked(openssl_key, &openssl_ks);
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
			ret = muggle_des_cfb64(&encrypt_ctx, plaintext, num_bytes, iv, &iv_off, ciphertext);
			ASSERT_EQ(ret, 0);

			// decrypt
			ret = muggle_des_cfb64(&decrypt_ctx, ciphertext, num_bytes, iv2, &iv_off2, ret_plaintext);
			ASSERT_EQ(ret, 0);

			ret = memcmp(plaintext, ret_plaintext, num_bytes);
			ASSERT_EQ(ret, 0);

			ret = memcmp(iv, iv2, MUGGLE_DES_BLOCK_SIZE);
			ASSERT_EQ(ret, 0);

			ASSERT_EQ(iv_off, iv_off2);

#if MUGGLE_TEST_LINK_OPENSSL
			openssl_ciphertext = (unsigned char*)openssl_ciphertext_arr + offset;
			DES_cfb64_encrypt(plaintext, openssl_ciphertext,
				(long)num_bytes, &openssl_ks, (DES_cblock*)iv3,
				&iv_off3, 1);

			ret = memcmp(ciphertext, openssl_ciphertext, num_bytes);
			if (ret != 0)
			{
				printf("muggle ciphertext: ");
				muggle_output_hex(ciphertext, num_bytes, 0);
				printf("openssl ciphertext: ");
				muggle_output_hex(openssl_ciphertext, num_bytes, 0);
			}
			ASSERT_EQ(ret, 0);

			ret = memcmp(iv, iv3, MUGGLE_DES_BLOCK_SIZE);
			ASSERT_EQ(ret, 0);

			ASSERT_EQ(iv_off, (unsigned int)iv_off3);
#endif

			offset += num_bytes;
		}
	}
}

TEST(crypt_des, ofb64)
{
	int ret;
	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE], iv2[MUGGLE_DES_BLOCK_SIZE], iv3[MUGGLE_DES_BLOCK_SIZE];
	unsigned int iv_off = 0, iv_off2 = 0;
	unsigned char plaintext_arr[TEST_SPACE_SIZE], ciphertext_arr[TEST_SPACE_SIZE], ret_plaintext_arr[TEST_SPACE_SIZE];
	unsigned char *plaintext, *ciphertext, *ret_plaintext;
	muggle_des_context_t encrypt_ctx, decrypt_ctx;
	unsigned int num_bytes = 0, remain_bytes = 0;
	int iv_off3 = 0;
	unsigned int offset = 0;
#if MUGGLE_TEST_LINK_OPENSSL
	unsigned char openssl_ciphertext_arr[TEST_SPACE_SIZE];
	unsigned char *openssl_ciphertext;
	DES_key_schedule openssl_ks;
#endif

	for (int i = 0; i < 16; ++i)
	{
		memset(plaintext_arr, 0, sizeof(plaintext_arr));
		memset(ciphertext_arr, 0, sizeof(ciphertext_arr));

		gen_input_var(key, iv, plaintext_arr, TEST_SPACE_SIZE);
		memcpy(iv2, iv, MUGGLE_DES_BLOCK_SIZE);
		memcpy(iv3, iv, MUGGLE_DES_BLOCK_SIZE);
		iv_off = iv_off2 = 0;
		iv_off3 = 0;
		offset = 0;

		// gen encrypt subkey
		ret = muggle_des_set_key(
			MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_OFB, key, &encrypt_ctx);
		ASSERT_EQ(ret, 0);

		// gen decrypt subkey
		ret = muggle_des_set_key(
			MUGGLE_DECRYPT, MUGGLE_BLOCK_CIPHER_MODE_OFB, key, &decrypt_ctx);
		ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
		const_DES_cblock *openssl_key = (const_DES_cblock*)&key;
		DES_set_key_unchecked(openssl_key, &openssl_ks);
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
			ret = muggle_des_ofb64(&encrypt_ctx, plaintext, num_bytes, iv, &iv_off, ciphertext);
			ASSERT_EQ(ret, 0);

			// decrypt
			ret = muggle_des_ofb64(&decrypt_ctx, ciphertext, num_bytes, iv2, &iv_off2, ret_plaintext);
			ASSERT_EQ(ret, 0);

			ret = memcmp(plaintext, ret_plaintext, num_bytes);
			ASSERT_EQ(ret, 0);

			ret = memcmp(iv, iv2, MUGGLE_DES_BLOCK_SIZE);
			ASSERT_EQ(ret, 0);

			ASSERT_EQ(iv_off, iv_off2);

#if MUGGLE_TEST_LINK_OPENSSL
			openssl_ciphertext = (unsigned char*)openssl_ciphertext_arr + offset;
			DES_ofb64_encrypt(plaintext, openssl_ciphertext,
				(long)num_bytes, &openssl_ks, (DES_cblock*)iv3,
				&iv_off3);

			ret = memcmp(ciphertext, openssl_ciphertext, num_bytes);
			if (ret != 0)
			{
				printf("muggle ciphertext: ");
				muggle_output_hex(ciphertext, num_bytes, 0);
				printf("openssl ciphertext: ");
				muggle_output_hex(openssl_ciphertext, num_bytes, 0);
			}
			ASSERT_EQ(ret, 0);

			ret = memcmp(iv, iv3, MUGGLE_DES_BLOCK_SIZE);
			ASSERT_EQ(ret, 0);

			ASSERT_EQ(iv_off, (unsigned int)iv_off3);
#endif

			offset += num_bytes;
		}
	}
}

TEST(crypt_des, ctr)
{
	int ret;
	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	uint64_t nonce, nonce2;
	unsigned int nonce_off = 0, nonce_off2 = 0;
	unsigned char plaintext_arr[TEST_SPACE_SIZE], ciphertext_arr[TEST_SPACE_SIZE], ret_plaintext_arr[TEST_SPACE_SIZE];
	unsigned char *plaintext, *ciphertext, *ret_plaintext;
	unsigned char stream_block[MUGGLE_DES_BLOCK_SIZE], stream_block2[MUGGLE_DES_BLOCK_SIZE];
	muggle_des_context_t encrypt_ctx, decrypt_ctx;
	unsigned int num_bytes = 0, remain_bytes = 0;
	unsigned int offset = 0;

	for (int i = 0; i < 16; ++i)
	{
		memset(plaintext_arr, 0, sizeof(plaintext_arr));
		memset(ciphertext_arr, 0, sizeof(ciphertext_arr));

		gen_input_var(key, (unsigned char*)&nonce, plaintext_arr, TEST_SPACE_SIZE);
		nonce2 = nonce;
		nonce_off = nonce_off2 = 0;
		offset = 0;

		// gen encrypt subkey
		ret = muggle_des_set_key(
			MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_CTR, key, &encrypt_ctx);
		ASSERT_EQ(ret, 0);

		// gen decrypt subkey
		ret = muggle_des_set_key(
			MUGGLE_DECRYPT, MUGGLE_BLOCK_CIPHER_MODE_CTR, key, &decrypt_ctx);
		ASSERT_EQ(ret, 0);

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
			ret = muggle_des_ctr(&encrypt_ctx, plaintext, num_bytes, &nonce, &nonce_off, stream_block, ciphertext);
			ASSERT_EQ(ret, 0);

			// decrypt
			ret = muggle_des_ctr(&decrypt_ctx, ciphertext, num_bytes, &nonce2, &nonce_off2, stream_block2, ret_plaintext);
			ASSERT_EQ(ret, 0);

			ret = memcmp(plaintext, ret_plaintext, num_bytes);
			ASSERT_EQ(ret, 0);

			ASSERT_EQ(nonce, nonce2);
			ASSERT_EQ(nonce_off, nonce_off2);

			ret = memcmp(stream_block, stream_block2, MUGGLE_DES_BLOCK_SIZE);
			ASSERT_EQ(ret, 0);

			offset += num_bytes;
		}
	}
}
