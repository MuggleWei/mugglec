#include "muggle/c/muggle_c.h"

#define EXAMPLE_MESSAGE_LEN 512

struct example_data
{
	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	union
	{
		unsigned char iv[MUGGLE_DES_BLOCK_SIZE];
		uint64_t      nonce;
	};
	unsigned char input[EXAMPLE_MESSAGE_LEN];
};

void gen_example_data(struct example_data *data)
{
	memset(data, 0, sizeof(struct example_data));

	srand((unsigned int)time(NULL));
	uint32_t r = 0;

	r = (uint32_t)rand();
	memcpy(&data->key[0], &r, 4);
	r = (uint32_t)rand();
	memcpy(&data->key[4], &r, 4);

	r = (uint32_t)rand();
	memcpy(&data->iv[0], &r, 4);
	r = (uint32_t)rand();
	memcpy(&data->iv[4], &r, 4);

	for (unsigned int i = 0; i < EXAMPLE_MESSAGE_LEN; ++i)
	{
		data->input[i] = (unsigned char)(rand() % 256);
	}
}

void output_err(int err, const char *msg)
{
	if (err == MUGGLE_ERR_NULL_PARAM)
	{
		MUGGLE_LOG_ERROR("[null parameter] %s", msg);
	}
	else if (err == MUGGLE_ERR_INVALID_PARAM)
	{
		MUGGLE_LOG_ERROR("[invalid parameter] %s", msg);
	}
	else
	{
		MUGGLE_LOG_ERROR("%s", msg);
	}

	muggle_print_stacktrace();
}

void example_des_single_block()
{
	printf("==========================\n");
	printf("DES encrypt/decrypt single block\n");

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
			muggle_des_context_t encrypt_ctx, decrypt_ctx;
			unsigned char ciphertext[MUGGLE_DES_BLOCK_SIZE], ret_plaintext[MUGGLE_DES_BLOCK_SIZE];

			// set encrypt/decrypt key
			int mode = MUGGLE_BLOCK_CIPHER_MODE_ECB;
			muggle_des_set_key(MUGGLE_ENCRYPT, mode, keys[i], &encrypt_ctx);
			muggle_des_set_key(MUGGLE_DECRYPT, mode, keys[i], &decrypt_ctx);

			// encrypt
			muggle_des_ecb(&encrypt_ctx, plaintexts[j], MUGGLE_DES_BLOCK_SIZE, ciphertext);

			// decrypt
			muggle_des_ecb(&decrypt_ctx, ciphertext, MUGGLE_DES_BLOCK_SIZE, ret_plaintext);

			printf("key: ");
			muggle_output_hex(keys[i], MUGGLE_DES_BLOCK_SIZE, 0);

			printf("plaintext: ");
			muggle_output_hex(plaintexts[j], MUGGLE_DES_BLOCK_SIZE, 0);

			printf("ciphertext: ");
			muggle_output_hex(ciphertext, MUGGLE_DES_BLOCK_SIZE, 0);

			printf("decrypt plaintext: ");
			muggle_output_hex(ret_plaintext, MUGGLE_DES_BLOCK_SIZE, 0);

			printf("\n");

			MUGGLE_ASSERT(memcmp(plaintexts[j], ret_plaintext, MUGGLE_DES_BLOCK_SIZE) == 0);
		}
	}
}

void example_des_ecb(struct example_data *data)
{
	printf("==========================\n");
	printf("DES encrypt/decrypt with ECB mode\n");

	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	unsigned char plaintext[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], ret_plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned int num_bytes = EXAMPLE_MESSAGE_LEN;
	muggle_des_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;

	memset(plaintext, 0, sizeof(plaintext));
	memset(ciphertext, 0, sizeof(ciphertext));
	memset(ret_plaintext, 0, sizeof(ret_plaintext));

	memcpy(key, data->key, MUGGLE_DES_BLOCK_SIZE);
	memcpy(plaintext, data->input, EXAMPLE_MESSAGE_LEN);

	// set encrypt/decrypt key
	int mode = MUGGLE_BLOCK_CIPHER_MODE_ECB;
	muggle_des_set_key(MUGGLE_ENCRYPT, mode, key, &encrypt_ctx);
	muggle_des_set_key(MUGGLE_DECRYPT, mode, key, &decrypt_ctx);

	// encrypt
	ret = muggle_des_ecb(&encrypt_ctx, plaintext, num_bytes, ciphertext);
	if (ret != 0)
	{
		output_err(ret, "failed encrypt with ecb mode");
		return;
	}

	// decrypt
	ret = muggle_des_ecb(&decrypt_ctx, ciphertext, num_bytes, ret_plaintext);
	if (ret != 0)
	{
		output_err(ret, "failed decrypt with ecb mode");
		return;
	}

	printf("key: ");
	muggle_output_hex(key, MUGGLE_DES_BLOCK_SIZE, 0);

	printf("plaintext: \n");
	muggle_output_hex(plaintext, num_bytes, 32);

	printf("ciphertext: \n");
	muggle_output_hex(ciphertext, num_bytes, 32);

	printf("decrypt plaintext: \n");
	muggle_output_hex(ret_plaintext, num_bytes, 32);

	MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, num_bytes) == 0);
}

void example_des_cbc(struct example_data *data)
{
	printf("==========================\n");
	printf("DES encrypt/decrypt with CBC mode\n");

	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE], iv2[MUGGLE_DES_BLOCK_SIZE];
	unsigned char plaintext[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], ret_plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned int num_bytes = EXAMPLE_MESSAGE_LEN;
	muggle_des_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;

	memset(plaintext, 0, sizeof(plaintext));
	memset(ciphertext, 0, sizeof(ciphertext));
	memset(ret_plaintext, 0, sizeof(ret_plaintext));

	memcpy(key, data->key, MUGGLE_DES_BLOCK_SIZE);
	memcpy(plaintext, data->input, EXAMPLE_MESSAGE_LEN);
	memcpy(iv, data->iv, MUGGLE_DES_BLOCK_SIZE);
	memcpy(iv2, iv, MUGGLE_DES_BLOCK_SIZE);

	// set encrypt/decrypt key
	int mode = MUGGLE_BLOCK_CIPHER_MODE_CBC;
	muggle_des_set_key(MUGGLE_ENCRYPT, mode, key, &encrypt_ctx);
	muggle_des_set_key(MUGGLE_DECRYPT, mode, key, &decrypt_ctx);

	// encrypt
	ret = muggle_des_cbc(&encrypt_ctx, plaintext, num_bytes, iv, ciphertext);
	if (ret != 0)
	{
		output_err(ret, "failed encrypt with cbc mode");
		return;
	}

	// decrypt
	ret = muggle_des_cbc(&decrypt_ctx, ciphertext, num_bytes, iv2, ret_plaintext);
	if (ret != 0)
	{
		output_err(ret, "failed decrypt with cbc mode");
		return;
	}

	printf("key: ");
	muggle_output_hex(key, MUGGLE_DES_BLOCK_SIZE, 0);

	printf("plaintext: \n");
	muggle_output_hex(plaintext, num_bytes, 32);

	printf("iv: \n");
	muggle_output_hex(data->iv, num_bytes, 32);

	printf("ciphertext: \n");
	muggle_output_hex(ciphertext, num_bytes, 32);

	printf("decrypt plaintext: \n");
	muggle_output_hex(ret_plaintext, num_bytes, 32);

	MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, num_bytes) == 0);
}

void example_des_cfb64(struct example_data *data)
{
	printf("==========================\n");
	printf("DES encrypt/decrypt with CFB 64bit iv mode\n");

	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE], iv2[MUGGLE_DES_BLOCK_SIZE];
	unsigned int iv_off = 0, iv_off2 = 0;
	unsigned char plaintext[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], ret_plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned int num_bytes = 0;
	muggle_des_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;
	unsigned int step = rand() % 128 + 1;
	unsigned int remain = EXAMPLE_MESSAGE_LEN;
	unsigned int offset = 0;
	const unsigned int total_msg_len = EXAMPLE_MESSAGE_LEN;

	memset(plaintext, 0, sizeof(plaintext));
	memset(ciphertext, 0, sizeof(ciphertext));
	memset(ret_plaintext, 0, sizeof(ret_plaintext));

	memcpy(key, data->key, MUGGLE_DES_BLOCK_SIZE);
	memcpy(plaintext, data->input, EXAMPLE_MESSAGE_LEN);
	memcpy(iv, data->iv, MUGGLE_DES_BLOCK_SIZE);
	memcpy(iv2, iv, MUGGLE_DES_BLOCK_SIZE);

	// set encrypt/decrypt key
	int mode = MUGGLE_BLOCK_CIPHER_MODE_CFB;
	muggle_des_set_key(MUGGLE_ENCRYPT, mode, key, &encrypt_ctx);
	muggle_des_set_key(MUGGLE_DECRYPT, mode, key, &decrypt_ctx);

	printf("key: ");
	muggle_output_hex(key, MUGGLE_DES_BLOCK_SIZE, 0);

	printf("plaintext: \n");
	muggle_output_hex(plaintext, total_msg_len, 32);

	printf("iv: \n");
	muggle_output_hex(data->iv, MUGGLE_DES_BLOCK_SIZE, 32);

	while (remain > 0)
	{
		num_bytes = step > remain ? remain : step;

		printf("encrypt iv: ");
		muggle_output_hex(iv, MUGGLE_DES_BLOCK_SIZE, 0);

		printf("decrypt iv: ");
		muggle_output_hex(iv2, MUGGLE_DES_BLOCK_SIZE, 0);

		// encrypt
		ret = muggle_des_cfb64(&encrypt_ctx, &plaintext[offset], num_bytes, iv, &iv_off, &ciphertext[offset]);
		if (ret != 0)
		{
			output_err(ret, "failed encrypt with cfb64 mode");
			return;
		}

		// decrypt
		ret = muggle_des_cfb64(&decrypt_ctx, &ciphertext[offset], num_bytes, iv2, &iv_off2, &ret_plaintext[offset]);
		if (ret != 0)
		{
			output_err(ret, "failed decrypt with cfb64 mode");
			return;
		}

		offset += num_bytes;
		remain -= num_bytes;

		printf("encrypt/decrypt %u bytes\n", num_bytes);

		printf("ciphertext: \n");
		muggle_output_hex(ciphertext, total_msg_len, 32);

		printf("decrypt plaintext: \n");
		muggle_output_hex(ret_plaintext, total_msg_len, 32);


		MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, offset) == 0);
	}


	MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, total_msg_len) == 0);
}

void example_des_ofb64(struct example_data *data)
{
	printf("==========================\n");
	printf("DES encrypt/decrypt with OFB 64bit iv mode\n");

	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	unsigned char iv[MUGGLE_DES_BLOCK_SIZE], iv2[MUGGLE_DES_BLOCK_SIZE];
	unsigned int iv_off = 0, iv_off2 = 0;
	unsigned char plaintext[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], ret_plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned int num_bytes = 0;
	muggle_des_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;
	unsigned int step = rand() % 128 + 1;
	unsigned int remain = EXAMPLE_MESSAGE_LEN;
	unsigned int offset = 0;
	const unsigned int total_msg_len = EXAMPLE_MESSAGE_LEN;

	memset(plaintext, 0, sizeof(plaintext));
	memset(ciphertext, 0, sizeof(ciphertext));
	memset(ret_plaintext, 0, sizeof(ret_plaintext));

	memcpy(key, data->key, MUGGLE_DES_BLOCK_SIZE);
	memcpy(plaintext, data->input, EXAMPLE_MESSAGE_LEN);
	memcpy(iv, data->iv, MUGGLE_DES_BLOCK_SIZE);
	memcpy(iv2, iv, MUGGLE_DES_BLOCK_SIZE);

	// set encrypt/decrypt key
	int mode = MUGGLE_BLOCK_CIPHER_MODE_OFB;
	muggle_des_set_key(MUGGLE_ENCRYPT, mode, key, &encrypt_ctx);
	muggle_des_set_key(MUGGLE_DECRYPT, mode, key, &decrypt_ctx);

	printf("key: ");
	muggle_output_hex(key, MUGGLE_DES_BLOCK_SIZE, 0);

	printf("plaintext: \n");
	muggle_output_hex(plaintext, total_msg_len, 32);

	while (remain > 0)
	{
		num_bytes = step > remain ? remain : step;

		printf("encrypt iv: ");
		muggle_output_hex(iv, MUGGLE_DES_BLOCK_SIZE, 0);

		printf("decrypt iv: ");
		muggle_output_hex(iv2, MUGGLE_DES_BLOCK_SIZE, 0);

		// encrypt
		ret = muggle_des_ofb64(&encrypt_ctx, &plaintext[offset], num_bytes, iv, &iv_off, &ciphertext[offset]);
		if (ret != 0)
		{
			output_err(ret, "failed encrypt with ofb64 mode");
			return;
		}

		// decrypt
		ret = muggle_des_ofb64(&decrypt_ctx, &ciphertext[offset], num_bytes, iv2, &iv_off2, &ret_plaintext[offset]);
		if (ret != 0)
		{
			output_err(ret, "failed decrypt with ofb64 mode");
			return;
		}

		offset += num_bytes;
		remain -= num_bytes;

		printf("encrypt/decrypt %u bytes\n", num_bytes);

		printf("ciphertext: \n");
		muggle_output_hex(ciphertext, total_msg_len, 32);

		printf("decrypt plaintext: \n");
		muggle_output_hex(ret_plaintext, total_msg_len, 32);


		MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, offset) == 0);
	}


	MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, total_msg_len) == 0);
}

void example_des_ctr(struct example_data *data)
{
	printf("==========================\n");
	printf("DES encrypt/decrypt with CTR iv mode\n");

	unsigned char key[MUGGLE_DES_BLOCK_SIZE];
	uint64_t nonce, nonce2;
	unsigned int nonce_off = 0, nonce_off2 = 0;
	unsigned char plaintext[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], ret_plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned char stream_block[MUGGLE_DES_BLOCK_SIZE], stream_block2[MUGGLE_DES_BLOCK_SIZE];
	unsigned int num_bytes = 0;
	muggle_des_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;
	unsigned int step = rand() % 128 + 1;
	unsigned int remain = EXAMPLE_MESSAGE_LEN;
	unsigned int offset = 0;
	const unsigned int total_msg_len = EXAMPLE_MESSAGE_LEN;

	memset(plaintext, 0, sizeof(plaintext));
	memset(ciphertext, 0, sizeof(ciphertext));
	memset(ret_plaintext, 0, sizeof(ret_plaintext));

	memcpy(key, data->key, MUGGLE_DES_BLOCK_SIZE);
	memcpy(plaintext, data->input, EXAMPLE_MESSAGE_LEN);
	nonce = data->nonce;
	nonce2 = nonce;
	nonce_off = nonce_off2 = 0;

	// set encrypt/decrypt key
	int mode = MUGGLE_BLOCK_CIPHER_MODE_CTR;
	muggle_des_set_key(MUGGLE_ENCRYPT, mode, key, &encrypt_ctx);
	muggle_des_set_key(MUGGLE_DECRYPT, mode, key, &decrypt_ctx);

	printf("key: ");
	muggle_output_hex(key, MUGGLE_DES_BLOCK_SIZE, 0);

	printf("plaintext: \n");
	muggle_output_hex(plaintext, total_msg_len, 32);

	while (remain > 0)
	{
		num_bytes = step > remain ? remain : step;

		printf("nonce: %llu\n", (unsigned long long)nonce);

		// encrypt
		ret = muggle_des_ctr(&encrypt_ctx, &plaintext[offset], num_bytes, &nonce, &nonce_off, stream_block, &ciphertext[offset]);
		if (ret != 0)
		{
			output_err(ret, "failed encrypt with ctr mode");
			return;
		}

		// decrypt
		ret = muggle_des_ctr(&decrypt_ctx, &ciphertext[offset], num_bytes, &nonce2, &nonce_off2, stream_block2, &ret_plaintext[offset]);
		if (ret != 0)
		{
			output_err(ret, "failed decrypt with ctr mode");
			return;
		}

		offset += num_bytes;
		remain -= num_bytes;

		printf("encrypt/decrypt %u bytes\n", num_bytes);

		printf("ciphertext: \n");
		muggle_output_hex(ciphertext, total_msg_len, 32);

		printf("decrypt plaintext: \n");
		muggle_output_hex(ret_plaintext, total_msg_len, 32);


		MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, offset) == 0);
	}

	MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, total_msg_len) == 0);
}

int main()
{
 	// simple init log
 	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
 	{
 		MUGGLE_LOG_ERROR("failed initialize log");
 		exit(EXIT_FAILURE);
 	}

	struct example_data data;
	gen_example_data(&data);

	// des crypt single block
	example_des_single_block();

	// des ecb mode
	example_des_ecb(&data);

	// des cbc mode
	example_des_cbc(&data);

	// des cfb64 mode
	example_des_cfb64(&data);

	// des ofb64 mode
	example_des_ofb64(&data);

	// des ctr mode
	example_des_ctr(&data);

	return 0;
}
