#include "muggle/c/muggle_c.h"
#include "muggle/c/crypt/openssl/openssl_des.h"

const char *block_cipher_mode_names[] = {
	"ECB", "CBC", "CFB", "OFB", "CTR"
};

void output_subkey(muggle_des_subkey_t *sk)
{
	unsigned char bytes[6];
	bytes[0] = (sk->bytes[0] & 0x3f) | ((sk->bytes[1] << 6) & 0xc0);
	bytes[1] = ((sk->bytes[1] >> 2) & 0x0f) | ((sk->bytes[2] << 4) & 0xf0);
	bytes[2] = ((sk->bytes[2] >> 4) & 0x03) | ((sk->bytes[3] << 2) & 0xfc);
	bytes[3] = (sk->bytes[4] & 0x3f) | ((sk->bytes[5] << 6) & 0xc0);
	bytes[4] = ((sk->bytes[5] >> 2) & 0x0f) | ((sk->bytes[6] << 4) & 0xf0);
	bytes[5] = ((sk->bytes[6] >> 4) & 0x03) | ((sk->bytes[7] << 2) & 0xfc);
	muggle_output_hex(bytes, 6, 0);
}

void example_des_block()
{
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

	for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
	{
		for (size_t j = 0; j < sizeof(plaintexts) / sizeof(plaintexts[0]); j++)
		{
			printf("============================\n");
			muggle_64bit_block_t key;
			muggle_des_subkeys_t ks;
			muggle_64bit_block_t plaintext, ciphertext, decrypt_ret;

			// key
			memcpy(key.bytes, keys[i], 8);

			// plaintext
			memcpy(plaintext.bytes, plaintexts[j], 8);

			printf("key: ");
			muggle_output_hex(key.bytes, 8, 0);

			printf("input plaintext: ");
			muggle_output_hex(plaintext.bytes, 8, 0);

			// encrypt genkey
			muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key, &ks);

			// encrypt
			muggle_des_crypt(&plaintext, &ks, &ciphertext);
			printf("encryption ciphertext: ");
			muggle_output_hex(ciphertext.bytes, 8, 0);
			muggle_output_bin(ciphertext.bytes, 8, 8);

			// decrypt genkey
			muggle_des_gen_subkeys(MUGGLE_DECRYPT, &key, &ks);

			// decrypt
			muggle_des_crypt(&ciphertext, &ks, &decrypt_ret);
			printf("decryption plaintext: ");
			muggle_output_hex(decrypt_ret.bytes, 8, 0);
			// muggle_output_bin(decrypt_ret.bytes, 8, 8);
		}
	}
}

// void example_des_cipher(int mode)
// {
// 	printf("============================\n");
// 	printf("DES encrypt/decrypt %s mode\n", block_cipher_mode_names[mode]);
// 
// 	// key
// 	muggle_des_key_block key;
// 	key.u64 = 0xf125cce7abe32f7c;
// 	printf("key: ");
// 	muggle_output_hex((unsigned char*)key.bytes, 8, 0);
// 
// 	// iv
// 	muggle_64bit_block_t iv;
// 	iv.u64 = 688888;
// 	printf("iv: ");
// 	muggle_output_hex((unsigned char*)iv.bytes, 8, 0);
// 
// 	int cnt = 26;
// 	int space = ROUND_UP_POW_OF_2_MUL(cnt, 8);
// 	unsigned char *input = (unsigned char*)malloc(space);
// 	unsigned char *output = (unsigned char*)malloc(space);
// 	unsigned char *plaintext = (unsigned char*)malloc(space);
// 	memset(input, 0, space);
// 	memset(output, 0, space);
// 
// 	for (int i = 0; i < cnt; ++i)
// 	{
// 		// input[i] = (unsigned char)(rand() % 256);
// 		input[i] = (unsigned char)('a' + i);
// 	}
// 	printf("input plaintext: ");
// 	muggle_output_hex(input, space, 0);
// 
// 	// encrypt
// 	if (muggle_des_cipher(mode, MUGGLE_ENCRYPT, input, output, space, key, &iv, 1, NULL) != 0)
// 	{
// 		MUGGLE_LOG_ERROR("failed DES encryption with %s mode", block_cipher_mode_names[mode]);
// 		free(input);
// 		free(output);
// 		free(plaintext);
// 		return;
// 	}
// 
// 	printf("encryption ciphertext: ");
// 	muggle_output_hex(output, space, 0);
// 	printf("return iv: ");
// 	muggle_output_hex((unsigned char*)iv.bytes, 8, 0);
// 
// 	// decrypt
// 	iv.u64 = 688888;
// 	if (muggle_des_cipher(mode, MUGGLE_DECRYPT, output, plaintext, space, key, &iv, 1, NULL) != 0)
// 	{
// 		MUGGLE_LOG_ERROR("failed DES decryption with %s mode", block_cipher_mode_names[mode]);
// 		free(input);
// 		free(output);
// 		free(plaintext);
// 		return;
// 	}
// 	printf("decryption plaintext: ");
// 	muggle_output_hex(plaintext, space, 0);
// 	printf("return iv: ");
// 	muggle_output_hex((unsigned char*)iv.bytes, 8, 0);
// 
// 	free(input);
// 	free(output);
// 	free(plaintext);
// }

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

//	// encrypt/decrypt plaintext
//	example_des_cipher(MUGGLE_BLOCK_CIPHER_MODE_ECB);
//	example_des_cipher(MUGGLE_BLOCK_CIPHER_MODE_CBC);
//	example_des_cipher(MUGGLE_BLOCK_CIPHER_MODE_CFB);
//	example_des_cipher(MUGGLE_BLOCK_CIPHER_MODE_OFB);
//	example_des_cipher(MUGGLE_BLOCK_CIPHER_MODE_CTR);

	return 0;
}
