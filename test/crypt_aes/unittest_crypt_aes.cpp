#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#if MUGGLE_TEST_LINK_OPENSSL
#include "openssl/aes.h"
#endif

void gen_input_var(
	unsigned char key[32], // 256bit
	unsigned char iv[32], // 256bit
	unsigned char *input,
	unsigned int num_bytes)
{
	srand((unsigned int)time(NULL));

	for (int i = 0; i < 32; ++i)
	{
		key[i] = (unsigned char)(rand() % 256);
	}

	for (int i = 0; i < 32; ++i)
	{
		iv[i] = (unsigned char)(rand() % 256);
	}

	for (unsigned int i = 0; i < num_bytes; ++i)
	{
		input[i] = (unsigned char)(rand() % 256);
	}
}


TEST(crypt_aes, expansion_key_128)
{
	// fips-197 Appendix A - Key Expansion Examples
	// A.1 Expansion of a 128-bit Cipher Key
	int ret = 0;
	muggle_aes_context_t ctx;

	unsigned char key[16] = {
		0x2b, 0x7e, 0x15, 0x16,
		0x28, 0xae, 0xd2, 0xa6,
		0xab, 0xf7, 0x15, 0x88,
		0x09, 0xcf, 0x4f, 0x3c
	};
	uint32_t rk_word[44] = {
		0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c,
		0xa0fafe17, 0x88542cb1, 0x23a33939, 0x2a6c7605,
		0xf2c295f2, 0x7a96b943, 0x5935807a, 0x7359f67f,
		0x3d80477d, 0x4716fe3e, 0x1e237e44, 0x6d7a883b,
		0xef44a541, 0xa8525b7f, 0xb671253b, 0xdb0bad00,
		0xd4d1c6f8, 0x7c839d87, 0xcaf2b8bc, 0x11f915bc,
		0x6d88a37a, 0x110b3efd, 0xdbf98641, 0xca0093fd,
		0x4e54f70e, 0x5f5fc9f3, 0x84a64fb2, 0x4ea6dc4f,
		0xead27321, 0xb58dbad2, 0x312bf560, 0x7f8d292f,
		0xac7766f3, 0x19fadc21, 0x28d12941, 0x575c006e,
		0xd014f9a8, 0xc9ee2589, 0xe13f0cc8, 0xb6630ca6
	};

	ret = muggle_aes_set_key(MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, key, 128, &ctx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(ctx.sk.rounds, 10);
	for (int i = 0; i < 44; i++)
	{
		if (ctx.sk.rd_key[i] != rk_word[i])
		{
			printf("i=%d, rd_key=%x, word=%x\n", i, ctx.sk.rd_key[i], rk_word[i]);
		}
		ASSERT_EQ(ctx.sk.rd_key[i], rk_word[i]);
	}
}

TEST(crypt_aes, expansion_key_192)
{
	// fips-197 Appendix A - Key Expansion Examples
	// A.2 Expansion of a 192-bit Cipher Key
	int ret = 0;
	muggle_aes_context_t ctx;

	unsigned char key[24] = {
		0x8e, 0x73, 0xb0, 0xf7,
		0xda, 0x0e, 0x64, 0x52,
		0xc8, 0x10, 0xf3, 0x2b,
		0x80, 0x90, 0x79, 0xe5,
		0x62, 0xf8, 0xea, 0xd2,
		0x52, 0x2c, 0x6b, 0x7b
	};

	uint32_t rk_word[52] = {
		0x8e73b0f7, 0xda0e6452, 0xc810f32b, 0x809079e5, 0x62f8ead2, 0x522c6b7b,
		0xfe0c91f7, 0x2402f5a5, 0xec12068e, 0x6c827f6b, 0x0e7a95b9, 0x5c56fec2,
		0x4db7b4bd, 0x69b54118, 0x85a74796, 0xe92538fd, 0xe75fad44, 0xbb095386,
		0x485af057, 0x21efb14f, 0xa448f6d9, 0x4d6dce24, 0xaa326360, 0x113b30e6,
		0xa25e7ed5, 0x83b1cf9a, 0x27f93943, 0x6a94f767, 0xc0a69407, 0xd19da4e1,
		0xec1786eb, 0x6fa64971, 0x485f7032, 0x22cb8755, 0xe26d1352, 0x33f0b7b3,
		0x40beeb28, 0x2f18a259, 0x6747d26b, 0x458c553e, 0xa7e1466c, 0x9411f1df,
		0x821f750a, 0xad07d753, 0xca400538, 0x8fcc5006, 0x282d166a, 0xbc3ce7b5,
		0xe98ba06f, 0x448c773c, 0x8ecc7204, 0x01002202,
	};

	ret = muggle_aes_set_key(MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, key, 192, &ctx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(ctx.sk.rounds, 12);
	for (int i = 0; i < 52; i++)
	{
		if (ctx.sk.rd_key[i] != rk_word[i])
		{
			printf("i=%d, rd_key=%x, word=%x\n", i, ctx.sk.rd_key[i], rk_word[i]);
		}
		ASSERT_EQ(ctx.sk.rd_key[i], rk_word[i]);
	}
}

TEST(crypt_aes, expansion_key_256)
{
	// fips-197 Appendix A - Key Expansion Examples
	// Expansion of a 192-bit Cipher Key
	int ret = 0;
	muggle_aes_context_t ctx;

	unsigned char key[32] = {
		0x60, 0x3d, 0xeb, 0x10,
		0x15, 0xca, 0x71, 0xbe,
		0x2b, 0x73, 0xae, 0xf0,
		0x85, 0x7d, 0x77, 0x81,
		0x1f, 0x35, 0x2c, 0x07,
		0x3b, 0x61, 0x08, 0xd7,
		0x2d, 0x98, 0x10, 0xa3,
		0x09, 0x14, 0xdf, 0xf4
	};

	uint32_t rk_word[60] = {
		0x603deb10, 0x15ca71be, 0x2b73aef0, 0x857d7781, 0x1f352c07, 0x3b6108d7, 0x2d9810a3, 0x0914dff4,
		0x9ba35411, 0x8e6925af, 0xa51a8b5f, 0x2067fcde, 0xa8b09c1a, 0x93d194cd, 0xbe49846e, 0xb75d5b9a,
		0xd59aecb8, 0x5bf3c917, 0xfee94248, 0xde8ebe96, 0xb5a9328a, 0x2678a647, 0x98312229, 0x2f6c79b3,
		0x812c81ad, 0xdadf48ba, 0x24360af2, 0xfab8b464, 0x98c5bfc9, 0xbebd198e, 0x268c3ba7, 0x09e04214,
		0x68007bac, 0xb2df3316, 0x96e939e4, 0x6c518d80, 0xc814e204, 0x76a9fb8a, 0x5025c02d, 0x59c58239,
		0xde136967, 0x6ccc5a71, 0xfa256395, 0x9674ee15, 0x5886ca5d, 0x2e2f31d7, 0x7e0af1fa, 0x27cf73c3,
		0x749c47ab, 0x18501dda, 0xe2757e4f, 0x7401905a, 0xcafaaae3, 0xe4d59b34, 0x9adf6ace, 0xbd10190d,
		0xfe4890d1, 0xe6188d0b, 0x046df344, 0x706c631e, 
	};

	ret = muggle_aes_set_key(MUGGLE_ENCRYPT, MUGGLE_BLOCK_CIPHER_MODE_ECB, key, 256, &ctx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(ctx.sk.rounds, 14);
	for (int i = 0; i < 60; i++)
	{
		if (ctx.sk.rd_key[i] != rk_word[i])
		{
			printf("i=%d, rd_key=%x, word=%x\n", i, ctx.sk.rd_key[i], rk_word[i]);
		}
		ASSERT_EQ(ctx.sk.rd_key[i], rk_word[i]);
	}
}

TEST(crypt_aes, cipher_example)
{
	// fips-197 APPENDIX B – CIPHER EXAMPLE
	unsigned char input[] = {
		0x32, 0x43, 0xf6, 0xa8,
		0x88, 0x5a, 0x30, 0x8d,
		0x31, 0x31, 0x98, 0xa2,
		0xe0, 0x37, 0x07, 0x34
	};
	unsigned char key[] = {
		0x2b, 0x7e, 0x15, 0x16,
		0x28, 0xae, 0xd2, 0xa6,
		0xab, 0xf7, 0x15, 0x88,
		0x09, 0xcf, 0x4f, 0x3c
	};
	unsigned char output[16], ret_plaintext[16];

	unsigned char expect_state[16] = {
		0x39, 0x02, 0xdc, 0x19,
		0x25, 0xdc, 0x11, 0x6a,
		0x84, 0x09, 0x85, 0x0b,
		0x1d, 0xfb, 0x97, 0x32
	};
	unsigned char expect_output[16];
	expect_output[0]  = expect_state[0];
	expect_output[1]  = expect_state[4];
	expect_output[2]  = expect_state[8];
	expect_output[3]  = expect_state[12];
	expect_output[4]  = expect_state[1];
	expect_output[5]  = expect_state[5];
	expect_output[6]  = expect_state[9];
	expect_output[7]  = expect_state[13];
	expect_output[8]  = expect_state[2];
	expect_output[9]  = expect_state[6];
	expect_output[10] = expect_state[10];
	expect_output[11] = expect_state[14];
	expect_output[12] = expect_state[3];
	expect_output[13] = expect_state[7];
	expect_output[14] = expect_state[11];
	expect_output[15] = expect_state[15];

	int ret = 0;
	int mode = MUGGLE_BLOCK_CIPHER_MODE_ECB;

	muggle_aes_context_t encrypt_ctx;
	ret = muggle_aes_set_key(MUGGLE_ENCRYPT, mode, key, 128, &encrypt_ctx);
	ASSERT_EQ(ret, 0);

	ret = muggle_aes_ecb(&encrypt_ctx, input, 16, output);
	ASSERT_EQ(ret, 0);

	ret = memcmp(output, expect_output, 16);
	if (ret != 0)
	{
		printf("output: \n");
		muggle_output_hex(output, 16, 4);
		printf("expect output: \n");
		muggle_output_hex(expect_output, 16, 4);
	}
	ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
	AES_KEY openssl_rk;
	ret = AES_set_encrypt_key(key, 128, &openssl_rk);
	ASSERT_EQ(ret, 0);

	unsigned char openssl_output[16];
	AES_encrypt(input, openssl_output, &openssl_rk);

	ret = memcmp(output, openssl_output, 16);
	if (ret != 0)
	{
		printf("output: \n");
		muggle_output_hex(output, 16, 4);
		printf("openssl output: \n");
		muggle_output_hex(openssl_output, 16, 4);
	}
	ASSERT_EQ(ret, 0);
#endif

	muggle_aes_context_t decrypt_ctx;
	ret = muggle_aes_set_key(MUGGLE_DECRYPT, mode, key, 128, &decrypt_ctx);
	ASSERT_EQ(ret, 0);

	ret = muggle_aes_ecb(&decrypt_ctx, output, 16, ret_plaintext);
	ASSERT_EQ(ret, 0);

	ret = memcmp(ret_plaintext, input, 16);
	if (ret != 0)
	{
		printf("input: \n");
		muggle_output_hex(input, 16, 4);
		printf("ret plaintext: \n");
		muggle_output_hex(ret_plaintext, 16, 4);
	}
	ASSERT_EQ(ret, 0);
}

TEST(crypt_aes, crypt_single_block)
{
	unsigned char plaintext[16], ret_plaintext[16];
	unsigned char key[32];
	unsigned char iv[32];
	unsigned char ciphertext[16];
	int bit_sizes[] = {128, 192, 256};
	int mode = MUGGLE_BLOCK_CIPHER_MODE_ECB;
	muggle_aes_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;

	srand((unsigned int)time(NULL));
	for (int i = 0; i < 3; i++)
	{
		gen_input_var(key, iv, plaintext, sizeof(plaintext));

		ret = muggle_aes_set_key(MUGGLE_ENCRYPT, mode, key, bit_sizes[i], &encrypt_ctx);
		ASSERT_EQ(ret, 0);

		ret = muggle_aes_ecb(&encrypt_ctx, plaintext, sizeof(plaintext), ciphertext);
		ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
		AES_KEY openssl_rk;
		ret = AES_set_encrypt_key(key, bit_sizes[i], &openssl_rk);
		ASSERT_EQ(ret, 0);

		unsigned char openssl_ciphertext[16];
		AES_encrypt(plaintext, openssl_ciphertext, &openssl_rk);

		ret = memcmp(ciphertext, openssl_ciphertext, 16);
		if (ret != 0)
		{
			printf("key: \n");
			muggle_output_hex(key, bit_sizes[i] / 8, 0);

			printf("plaintext: \n");
			muggle_output_hex(plaintext, 16, 0);

			printf("ciphertext: \n");
			muggle_output_hex(ciphertext, 16, 0);

			printf("openssl output: \n");
			muggle_output_hex(openssl_ciphertext, 16, 0);
		}
		ASSERT_EQ(ret, 0);
#endif

		ret = muggle_aes_set_key(MUGGLE_DECRYPT, mode, key, bit_sizes[i], &decrypt_ctx);
		ASSERT_EQ(ret, 0);

		ret = muggle_aes_ecb(&decrypt_ctx, ciphertext, sizeof(ciphertext), ret_plaintext);
		ASSERT_EQ(ret, 0);

		ret = memcmp(ret_plaintext, plaintext, 16);
		if (ret != 0)
		{
			printf("plaintext: \n");
			muggle_output_hex(plaintext, 16, 0);

			printf("ciphertext: \n");
			muggle_output_hex(ciphertext, 16, 0);

			printf("key: \n");
			muggle_output_hex(key, bit_sizes[i] / 8, 0);

			printf("ret plaintext: \n");
			muggle_output_hex(ret_plaintext, 16, 0);
		}
		ASSERT_EQ(ret, 0);
	}
}
