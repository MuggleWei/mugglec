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

#define EXAMPLE_MESSAGE_LEN 512

void example_des_cipher(int block_cipher_mode)
{
	static const char* block_cipher_mode_names[MAX_MUGGLE_BLOCK_CIPHER_MODE] = {
		"ECB", "CBC", "CFB", "OFB", "CTR"
	};

	printf("============================\n");
	printf("DES encrypt/decrypt %s mode\n", block_cipher_mode_names[block_cipher_mode]);

	int ret;
	muggle_64bit_block_t key;
	muggle_64bit_block_t iv, iv2, iv_save;
	unsigned char input[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned int num_bytes = (unsigned int)sizeof(input);
	for (int i = 0; i < 16; ++i)
	{
		gen_input_var(&key, &iv, input, num_bytes);
		iv_save.u64 = iv2.u64 = iv.u64;

		muggle_des_cipher(block_cipher_mode, MUGGLE_ENCRYPT, key, input, num_bytes, &iv, 1, ciphertext);
		muggle_des_cipher(block_cipher_mode, MUGGLE_DECRYPT, key, ciphertext, num_bytes, &iv2, 1, plaintext);

		printf("key: ");
		muggle_output_hex((unsigned char*)key.bytes, 8, 0);
		printf("iv: ");
		muggle_output_hex((unsigned char*)iv_save.bytes, 8, 0);
		printf("input: \n");
		muggle_output_hex((unsigned char*)input, num_bytes, 16);
		printf("ciphertext: \n");
		muggle_output_hex((unsigned char*)ciphertext, num_bytes, 16);
		printf("plaintext: \n");
		muggle_output_hex((unsigned char*)plaintext, num_bytes, 16);

		ret = memcmp(input, plaintext, num_bytes);
		if (ret != 0)
		{
			MUGGLE_LOG_ERROR("DES failed: input plaintext != decryption plaintext");
			exit(EXIT_FAILURE);
		}
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

	srand((unsigned int)time(NULL));

	// crypt single block
	example_des_block();

	// encrypt/decrypt plaintext
	example_des_cipher(MUGGLE_BLOCK_CIPHER_MODE_ECB);
	example_des_cipher(MUGGLE_BLOCK_CIPHER_MODE_CBC);
	example_des_cipher(MUGGLE_BLOCK_CIPHER_MODE_CFB);
	example_des_cipher(MUGGLE_BLOCK_CIPHER_MODE_OFB);
	example_des_cipher(MUGGLE_BLOCK_CIPHER_MODE_CTR);

	return 0;
}
