#include "muggle/c/muggle_c.h"
#include "muggle/c/crypt/openssl/openssl_des.h"

#define EXAMPLE_MESSAGE_LEN 16

const char *block_cipher_mode_names[] = {
	"ECB", "CBC", "CFB", "OFB", "CTR"
};

void gen_input_var(
	muggle_64bit_block_t *key1, muggle_64bit_block_t *key2, muggle_64bit_block_t *key3,
	muggle_64bit_block_t *iv, unsigned char *input, unsigned int num_bytes)
{
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

void example_tdes_block()
{
	printf("============================\n");
	printf("TDES encrypt/decrypt single block\n");

	int ret;
	muggle_64bit_block_t key1, key2, key3;
	muggle_64bit_block_t iv, iv2, iv_save;
	unsigned char input[8];
	unsigned int num_bytes = (unsigned int)sizeof(input);

	for (int i = 0; i < 2; ++i)
	{
		printf("============================\n");
		gen_input_var(&key1, &key2, &key3, &iv, input, num_bytes);

		muggle_64bit_block_t input_block, output_block, decrypt_ret;
		muggle_des_subkeys_t ks1, ks2, ks3;
		memcpy(input_block.bytes, input, 8);

		printf("keys1: ");
		muggle_output_hex(key1.bytes, 8, 0);
		printf("keys2: ");
		muggle_output_hex(key2.bytes, 8, 0);
		printf("keys3: ");
		muggle_output_hex(key3.bytes, 8, 0);

		printf("input plaintext: ");
		muggle_output_hex(input_block.bytes, 8, 0);

		// encrypt subkeys
		muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key1, &ks1);
		muggle_des_gen_subkeys(MUGGLE_DECRYPT, &key2, &ks2);
		muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key3, &ks3);

		// encrypt
		muggle_tdes_crypt(&input_block, &ks1, &ks2, &ks3, &output_block);
		printf("encryption ciphertext: ");
		muggle_output_hex(output_block.bytes, 8, 0);
		// muggle_output_bin(output_block.bytes, 8, 8);

		// decrypt subkeys
		muggle_des_gen_subkeys(MUGGLE_DECRYPT, &key1, &ks1);
		muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key2, &ks2);
		muggle_des_gen_subkeys(MUGGLE_DECRYPT, &key3, &ks3);

		// decrypt
		muggle_tdes_crypt(&output_block, &ks1, &ks2, &ks3, &decrypt_ret);
		printf("decryption plaintext: ");
		muggle_output_hex(decrypt_ret.bytes, 8, 0);

		ret = memcmp(input_block.bytes, decrypt_ret.bytes, 8);
		if (ret != 0)
		{
			MUGGLE_LOG_ERROR("TDES failed: input plaintext != decryption plaintext");
			exit(EXIT_FAILURE);
		}
	}
}

void example_tdes_cipher(int block_cipher_mode)
{
	printf("============================\n");
	printf("TDES encrypt/decrypt %s mode\n", block_cipher_mode_names[block_cipher_mode]);

	int ret;
	muggle_64bit_block_t key1, key2, key3;
	muggle_64bit_block_t iv, iv2, iv_save;
	unsigned char input[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned int num_bytes = (unsigned int)sizeof(input);

	for (int i = 0; i < 16; ++i)
	{
		printf("============================\n");
		gen_input_var(&key1, &key2, &key3, &iv, input, num_bytes);
		iv_save.u64 = iv2.u64 = iv.u64;

		muggle_tdes_cipher(block_cipher_mode, MUGGLE_ENCRYPT,
			key1, key2, key3, input, num_bytes, &iv, 1, ciphertext);
		muggle_tdes_cipher(block_cipher_mode, MUGGLE_DECRYPT,
			key1, key2, key3, ciphertext, num_bytes, &iv2, 1, plaintext);

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

		ret = memcmp(input, plaintext, EXAMPLE_MESSAGE_LEN);
		if (ret != 0)
		{
			MUGGLE_LOG_ERROR("TDES failed: input plaintext != decryption plaintext");
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
	example_tdes_block();

	// encrypt/decrypt plaintext
	example_tdes_cipher(MUGGLE_BLOCK_CIPHER_MODE_ECB);
	example_tdes_cipher(MUGGLE_BLOCK_CIPHER_MODE_CBC);
	example_tdes_cipher(MUGGLE_BLOCK_CIPHER_MODE_CFB);
	example_tdes_cipher(MUGGLE_BLOCK_CIPHER_MODE_OFB);
	example_tdes_cipher(MUGGLE_BLOCK_CIPHER_MODE_CTR);

	return 0;
}
