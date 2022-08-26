#include "muggle/c/muggle_c.h"

#define EXAMPLE_MESSAGE_LEN 512

struct example_data
{
	unsigned char key[3][32]; // 128|192|256 bits
	union
	{
		unsigned char iv[MUGGLE_AES_BLOCK_SIZE];
		uint64_t      nonce[2];
	};
	unsigned char input[EXAMPLE_MESSAGE_LEN];
};

void gen_example_data(struct example_data *data)
{
	memset(data, 0, sizeof(struct example_data));

	srand((unsigned int)time(NULL));
	uint32_t r = 0;

	for (unsigned int j = 0; j < 3; ++j)
	{
		for (unsigned int i = 0; i < 32; i++)
		{
			data->key[j][i] = (unsigned char)(rand() % 256);
		}
	}

	for (unsigned int i = 0; i < (unsigned int)sizeof(data->iv); i++)
	{
		data->iv[i] = (unsigned char)(rand() % 256);
	}

	for (unsigned int i = 0; i < (unsigned int)sizeof(data->input); i++)
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

void example_aes_ecb(struct example_data *data)
{
	printf("==========================\n");
	printf("AES encrypt/decrypt with ECB mode\n");

	const int bit_sizes[] = {128, 192, 256};
	unsigned char key[32];
	unsigned char plaintext[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], ret_plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned int num_bytes = EXAMPLE_MESSAGE_LEN;
	muggle_aes_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;

	memset(plaintext, 0, sizeof(plaintext));
	memcpy(plaintext, data->input, EXAMPLE_MESSAGE_LEN);

	for (int i = 0; i < 3; i++)
	{
		memset(ciphertext, 0, sizeof(ciphertext));
		memset(ret_plaintext, 0, sizeof(ret_plaintext));

		memcpy(key, data->key[i], bit_sizes[i] / 8);

		// set encrypt/decrypt key
		int mode = MUGGLE_BLOCK_CIPHER_MODE_ECB;
		ret = muggle_aes_set_key(MUGGLE_ENCRYPT, mode, key, bit_sizes[i], &encrypt_ctx);
		if (ret != 0)
		{
			output_err(ret, "failed AES ecb set encrypt key");
			continue;
		}

		ret = muggle_aes_set_key(MUGGLE_DECRYPT, mode, key, bit_sizes[i], &decrypt_ctx);
		if (ret != 0)
		{
			output_err(ret, "failed AES ecb set decrypt key");
			continue;
		}

		// encrypt
		ret = muggle_aes_ecb(&encrypt_ctx, plaintext, num_bytes, ciphertext);
		if (ret != 0)
		{
			output_err(ret, "failed AES ecb encrypt");
			continue;
		}

		// decrypt
		ret = muggle_aes_ecb(&decrypt_ctx, ciphertext, num_bytes, ret_plaintext);
		if (ret != 0)
		{
			output_err(ret, "failed AES ecb decrypt");
			continue;
		}

		printf("key[%d bits]: ", bit_sizes[i]);
		muggle_output_hex(key, bit_sizes[i] / 8, 0);

		printf("plaintext: \n");
		muggle_output_hex(plaintext, num_bytes, 32);

		printf("ciphertext: \n");
		muggle_output_hex(ciphertext, num_bytes, 32);

		printf("decrypt plaintext: \n");
		muggle_output_hex(ret_plaintext, num_bytes, 32);

		MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, num_bytes) == 0);
	}
}

void example_aes_cbc(struct example_data *data)
{
	printf("==========================\n");
	printf("AES encrypt/decrypt with CBC mode\n");

	const int bit_sizes[] = {128, 192, 256};
	unsigned char key[32];
	unsigned char iv[MUGGLE_AES_BLOCK_SIZE], iv2[MUGGLE_AES_BLOCK_SIZE];
	unsigned char plaintext[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], ret_plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned int num_bytes = EXAMPLE_MESSAGE_LEN;
	muggle_aes_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;

	memset(plaintext, 0, sizeof(plaintext));
	memcpy(plaintext, data->input, EXAMPLE_MESSAGE_LEN);

	for (int i = 0; i < 3; i++)
	{
		memset(ciphertext, 0, sizeof(ciphertext));
		memset(ret_plaintext, 0, sizeof(ret_plaintext));

		memcpy(key, data->key[i], bit_sizes[i] / 8);
		memcpy(iv, data->iv, MUGGLE_AES_BLOCK_SIZE);
		memcpy(iv2, iv, MUGGLE_AES_BLOCK_SIZE);

		// set encrypt/decrypt key
		int mode = MUGGLE_BLOCK_CIPHER_MODE_CBC;
		ret = muggle_aes_set_key(MUGGLE_ENCRYPT, mode, key, bit_sizes[i], &encrypt_ctx);
		if (ret != 0)
		{
			output_err(ret, "failed AES cbc set encrypt key");
			continue;
		}

		ret = muggle_aes_set_key(MUGGLE_DECRYPT, mode, key, bit_sizes[i], &decrypt_ctx);
		if (ret != 0)
		{
			output_err(ret, "failed AES cbc set decrypt key");
			continue;
		}

		// encrypt
		ret = muggle_aes_cbc(&encrypt_ctx, plaintext, num_bytes, iv, ciphertext);
		if (ret != 0)
		{
			output_err(ret, "failed AES cbc encrypt");
			continue;
		}

		// decrypt
		ret = muggle_aes_cbc(&decrypt_ctx, ciphertext, num_bytes, iv2, ret_plaintext);
		if (ret != 0)
		{
			output_err(ret, "failed AES cbc decrypt");
			continue;
		}

		printf("key[%d bits]: ", bit_sizes[i]);
		muggle_output_hex(key, bit_sizes[i] / 8, 0);

		printf("iv: ");
		muggle_output_hex(data->iv, MUGGLE_AES_BLOCK_SIZE, 0);

		printf("plaintext: \n");
		muggle_output_hex(plaintext, num_bytes, 32);

		printf("ciphertext: \n");
		muggle_output_hex(ciphertext, num_bytes, 32);

		printf("decrypt plaintext: \n");
		muggle_output_hex(ret_plaintext, num_bytes, 32);

		MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, num_bytes) == 0);
	}
}

void example_aes_cfb128(struct example_data *data)
{
	printf("==========================\n");
	printf("AES encrypt/decrypt with CFB128 mode\n");

	const int bit_sizes[] = {128, 192, 256};
	unsigned char key[32];
	unsigned char iv[MUGGLE_AES_BLOCK_SIZE], iv2[MUGGLE_AES_BLOCK_SIZE];
	unsigned int iv_off = 0, iv_off2 = 0;
	unsigned char plaintext[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], ret_plaintext[EXAMPLE_MESSAGE_LEN];
	muggle_aes_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;
	unsigned int step = rand() % 128 + 1;
	unsigned int num_bytes = EXAMPLE_MESSAGE_LEN;
	unsigned int remain = EXAMPLE_MESSAGE_LEN;
	unsigned int offset = 0;
	const unsigned int total_msg_len = EXAMPLE_MESSAGE_LEN;

	memset(plaintext, 0, sizeof(plaintext));
	memcpy(plaintext, data->input, EXAMPLE_MESSAGE_LEN);

	for (int i = 0; i < 3; i++)
	{
		memset(ciphertext, 0, sizeof(ciphertext));
		memset(ret_plaintext, 0, sizeof(ret_plaintext));

		memcpy(key, data->key[i], bit_sizes[i] / 8);
		memcpy(iv, data->iv, MUGGLE_AES_BLOCK_SIZE);
		memcpy(iv2, iv, MUGGLE_AES_BLOCK_SIZE);

		iv_off = 0;
		iv_off2 = 0;

		remain = EXAMPLE_MESSAGE_LEN;
		offset = 0;

		// set encrypt/decrypt key
		int mode = MUGGLE_BLOCK_CIPHER_MODE_CFB;
		ret = muggle_aes_set_key(MUGGLE_ENCRYPT, mode, key, bit_sizes[i], &encrypt_ctx);
		if (ret != 0)
		{
			output_err(ret, "failed AES cfb128 set encrypt key");
			continue;
		}

		ret = muggle_aes_set_key(MUGGLE_DECRYPT, mode, key, bit_sizes[i], &decrypt_ctx);
		if (ret != 0)
		{
			output_err(ret, "failed AES cfb128 set decrypt key");
			continue;
		}

		printf("key[%d bits]: ", bit_sizes[i]);
		muggle_output_hex(key, bit_sizes[i] / 8, 0);

		while (remain > 0)
		{
			num_bytes = step > remain ? remain : step;

			// encrypt
			ret = muggle_aes_cfb128(&encrypt_ctx, &plaintext[offset], num_bytes, iv, &iv_off, &ciphertext[offset]);
			if (ret != 0)
			{
				output_err(ret, "failed AES cfb128 encrypt");
				continue;
			}

			// decrypt
			ret = muggle_aes_cfb128(&decrypt_ctx, &ciphertext[offset], num_bytes, iv2, &iv_off2, &ret_plaintext[offset]);
			if (ret != 0)
			{
				output_err(ret, "failed AES cfb128 decrypt");
				continue;
			}

			offset += num_bytes;
			remain -= num_bytes;

			printf("plaintext: \n");
			muggle_output_hex(plaintext, total_msg_len, 32);

			printf("ciphertext: \n");
			muggle_output_hex(ciphertext, total_msg_len, 32);

			printf("decrypt plaintext: \n");
			muggle_output_hex(ret_plaintext, total_msg_len, 32);

			MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, offset) == 0);
		}

		MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, total_msg_len) == 0);
	}
}

void example_aes_ofb128(struct example_data *data)
{
	printf("==========================\n");
	printf("AES encrypt/decrypt with OFB128 mode\n");

	const int bit_sizes[] = {128, 192, 256};
	unsigned char key[32];
	unsigned char iv[MUGGLE_AES_BLOCK_SIZE], iv2[MUGGLE_AES_BLOCK_SIZE];
	unsigned int iv_off = 0, iv_off2 = 0;
	unsigned char plaintext[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], ret_plaintext[EXAMPLE_MESSAGE_LEN];
	muggle_aes_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;
	unsigned int step = rand() % 128 + 1;
	unsigned int num_bytes = EXAMPLE_MESSAGE_LEN;
	unsigned int remain = EXAMPLE_MESSAGE_LEN;
	unsigned int offset = 0;
	const unsigned int total_msg_len = EXAMPLE_MESSAGE_LEN;

	memset(plaintext, 0, sizeof(plaintext));
	memcpy(plaintext, data->input, EXAMPLE_MESSAGE_LEN);

	for (int i = 0; i < 3; i++)
	{
		memset(ciphertext, 0, sizeof(ciphertext));
		memset(ret_plaintext, 0, sizeof(ret_plaintext));

		memcpy(key, data->key[i], bit_sizes[i] / 8);
		memcpy(iv, data->iv, MUGGLE_AES_BLOCK_SIZE);
		memcpy(iv2, iv, MUGGLE_AES_BLOCK_SIZE);

		iv_off = 0;
		iv_off2 = 0;

		remain = EXAMPLE_MESSAGE_LEN;
		offset = 0;

		// set encrypt/decrypt key
		int mode = MUGGLE_BLOCK_CIPHER_MODE_OFB;
		ret = muggle_aes_set_key(MUGGLE_ENCRYPT, mode, key, bit_sizes[i], &encrypt_ctx);
		if (ret != 0)
		{
			output_err(ret, "failed AES ofb128 set encrypt key");
			continue;
		}

		ret = muggle_aes_set_key(MUGGLE_DECRYPT, mode, key, bit_sizes[i], &decrypt_ctx);
		if (ret != 0)
		{
			output_err(ret, "failed AES ofb128 set decrypt key");
			continue;
		}

		printf("key[%d bits]: ", bit_sizes[i]);
		muggle_output_hex(key, bit_sizes[i] / 8, 0);

		while (remain > 0)
		{
			num_bytes = step > remain ? remain : step;

			// encrypt
			ret = muggle_aes_ofb128(&encrypt_ctx, &plaintext[offset], num_bytes, iv, &iv_off, &ciphertext[offset]);
			if (ret != 0)
			{
				output_err(ret, "failed AES ofb128 encrypt");
				continue;
			}

			// decrypt
			ret = muggle_aes_ofb128(&decrypt_ctx, &ciphertext[offset], num_bytes, iv2, &iv_off2, &ret_plaintext[offset]);
			if (ret != 0)
			{
				output_err(ret, "failed AES ofb128 decrypt");
				continue;
			}

			offset += num_bytes;
			remain -= num_bytes;

			printf("plaintext: \n");
			muggle_output_hex(plaintext, total_msg_len, 32);

			printf("ciphertext: \n");
			muggle_output_hex(ciphertext, total_msg_len, 32);

			printf("decrypt plaintext: \n");
			muggle_output_hex(ret_plaintext, total_msg_len, 32);

			MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, offset) == 0);
		}

		MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, total_msg_len) == 0);
	}
}

void example_aes_ctr(struct example_data *data)
{
	printf("==========================\n");
	printf("AES encrypt/decrypt with CTR mode\n");

	const int bit_sizes[] = {128, 192, 256};
	unsigned char key[32];
	uint64_t nonce[2], nonce2[2];
	unsigned int nonce_off = 0, nonce_off2 = 0;
	unsigned char plaintext[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], ret_plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned char stream_block[MUGGLE_AES_BLOCK_SIZE], stream_block2[MUGGLE_AES_BLOCK_SIZE];
	muggle_aes_context_t encrypt_ctx, decrypt_ctx;
	int ret = 0;
	unsigned int step = rand() % 128 + 1;
	unsigned int num_bytes = 0;
	unsigned int remain = EXAMPLE_MESSAGE_LEN;
	unsigned int offset = 0;
	const unsigned int total_msg_len = EXAMPLE_MESSAGE_LEN;

	memset(plaintext, 0, sizeof(plaintext));
	memcpy(plaintext, data->input, EXAMPLE_MESSAGE_LEN);

	for (int i = 0; i < 3; i++)
	{
		memset(ciphertext, 0, sizeof(ciphertext));
		memset(ret_plaintext, 0, sizeof(ret_plaintext));
		memset(stream_block, 0, sizeof(stream_block));
		memset(stream_block2, 0, sizeof(stream_block2));

		memcpy(key, data->key[i], bit_sizes[i] / 8);
		memcpy(nonce, data->nonce, MUGGLE_AES_BLOCK_SIZE);
		memcpy(nonce2, data->nonce, MUGGLE_AES_BLOCK_SIZE);

		nonce_off = 0;
		nonce_off2 = 0;

		remain = EXAMPLE_MESSAGE_LEN;
		offset = 0;

		// set encrypt/decrypt key
		int mode = MUGGLE_BLOCK_CIPHER_MODE_CTR;
		ret = muggle_aes_set_key(MUGGLE_ENCRYPT, mode, key, bit_sizes[i], &encrypt_ctx);
		if (ret != 0)
		{
			output_err(ret, "failed AES ctr set encrypt key");
			continue;
		}

		ret = muggle_aes_set_key(MUGGLE_DECRYPT, mode, key, bit_sizes[i], &decrypt_ctx);
		if (ret != 0)
		{
			output_err(ret, "failed AES ctr set decrypt key");
			continue;
		}

		printf("key[%d bits]: ", bit_sizes[i]);
		muggle_output_hex(key, bit_sizes[i] / 8, 0);

		while (remain > 0)
		{
			num_bytes = step > remain ? remain : step;

			// encrypt
			ret = muggle_aes_ctr(&encrypt_ctx, &plaintext[offset], num_bytes, nonce, &nonce_off, stream_block, &ciphertext[offset]);
			if (ret != 0)
			{
				output_err(ret, "failed AES ctr encrypt");
				continue;
			}

			// decrypt
			ret = muggle_aes_ctr(&decrypt_ctx, &ciphertext[offset], num_bytes, nonce2, &nonce_off2, stream_block2, &ret_plaintext[offset]);
			if (ret != 0)
			{
				output_err(ret, "failed AES ctr decrypt");
				continue;
			}

			offset += num_bytes;
			remain -= num_bytes;

			printf("plaintext: \n");
			muggle_output_hex(plaintext, total_msg_len, 32);

			printf("ciphertext: \n");
			muggle_output_hex(ciphertext, total_msg_len, 32);

			printf("decrypt plaintext: \n");
			muggle_output_hex(ret_plaintext, total_msg_len, 32);

			MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, offset) == 0);
		}

		MUGGLE_ASSERT(memcmp(plaintext, ret_plaintext, total_msg_len) == 0);
	}
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

	// aes ecb mode
	example_aes_ecb(&data);

	// aes cbc mode
	example_aes_cbc(&data);

	// aes cfb128 mode
	example_aes_cfb128(&data);

	// aes ofb128 mode
	example_aes_ofb128(&data);

	// aes ctr mode
	example_aes_ctr(&data);

	return 0;
}
