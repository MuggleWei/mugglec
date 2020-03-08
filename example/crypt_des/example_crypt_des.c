#include "muggle/c/muggle_c.h"
#include "muggle/c/crypt/openssl/openssl_des.h"

void example_des_block()
{
	printf("============================\n");
	printf("DES encrypt/decrypt single block\n");

	// key
	muggle_des_key_block key;
	key.u64 = 0xf125cce7abe32f7c;
	printf("key: ");
	muggle_output_hex((unsigned char*)key.bytes, 8, 0);

	// generate subkey
	muggle_des_subkeys_t ks;
	muggle_openssl_des_set_key_unchecked((unsigned char*)key.bytes, &ks);

	// plaintext block
	muggle_64bit_block_t plaintext;
	for (int i = 0; i < 8; ++i)
	{
		plaintext.bytes[i] = 0x01 << i;
		// plaintext.bytes[i] = rand() % 256;
	}
	printf("plaintext: ");
	muggle_output_hex(plaintext.bytes, 8, 0);

	// encrypt
	muggle_64bit_block_t ciphertext;
	muggle_des_crypt(MUGGLE_ENCRYPT, &plaintext, &ciphertext, &ks, NULL);
	printf("encryption ciphertext: ");
	muggle_output_hex(ciphertext.bytes, 8, 0);

	// decrypt
	memset(&plaintext, 0, sizeof(plaintext));
	muggle_des_crypt(MUGGLE_DECRYPT, &ciphertext, &plaintext, &ks, NULL);
	printf("decryption plaintext: ");
	muggle_output_hex(plaintext.bytes, 8, 0);
}

void example_des_cbc()
{
	printf("============================\n");
	printf("DES encrypt/decrypt CBC mode\n");

	// key
	muggle_des_key_block key;
	key.u64 = 0xf125cce7abe32f7c;
	printf("key: ");
	muggle_output_hex((unsigned char*)key.bytes, 8, 0);

	// iv
	muggle_64bit_block_t iv;
	iv.u64 = 688888;
	printf("iv: ");
	muggle_output_hex((unsigned char*)iv.bytes, 8, 0);

	int cnt = 26;
	int space = ROUND_UP_POW_OF_2_MUL(cnt, 8);
	unsigned char *input = (unsigned char*)malloc(space);
	unsigned char *output = (unsigned char*)malloc(space);
	unsigned char *plaintext = (unsigned char*)malloc(space);
	memset(input, 0, space);
	memset(output, 0, space);

	for (int i = 0; i < cnt; ++i)
	{
		input[i] = (unsigned char)('a' + i);
	}
	printf("plaintext: ");
	muggle_output_hex(input, space, 0);

	// des with cbc encrypt
	if (muggle_des_cbc(MUGGLE_ENCRYPT, input, output, space, key, &iv, 1, NULL) != 0)
	{
		MUGGLE_LOG_ERROR("failed DES encryption with CBC mode");
		free(input);
		free(output);
		free(plaintext);
		return;
	}
	printf("ciphertext: ");
	muggle_output_hex(output, space, 0);
	printf("return iv: ");
	muggle_output_hex((unsigned char*)iv.bytes, 8, 0);

	// des with cbc decrypt
	iv.u64 = 688888;
	if (muggle_des_cbc(MUGGLE_DECRYPT, output, plaintext, space, key, &iv, 1, NULL) != 0)
	{
		MUGGLE_LOG_ERROR("failed DES decryption with CBC mode");
		free(input);
		free(output);
		free(plaintext);
		return;
	}
	printf("return plaintext: ");
	muggle_output_hex(plaintext, space, 0);
	printf("return iv: ");
	muggle_output_hex((unsigned char*)iv.bytes, 8, 0);

	free(input);
	free(output);
	free(plaintext);
}

int main()
{
	// simple init log
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed initialize log");
		exit(EXIT_FAILURE);
	}

	srand((unsigned int)time(NULL));

	// crypt single block
	example_des_block();

	// cbc mode
	example_des_cbc();

	return 0;
}
