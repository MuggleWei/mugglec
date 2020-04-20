#include "muggle/c/muggle_c.h"
#include "muggle/c/crypt/openssl/openssl_des.h"

int main()
{
	return 0;
}

// const char *block_cipher_mode_names[] = {
// 	"ECB", "CBC", "CFB", "OFB", "CTR"
// };
// 
// void example_tdes_block()
// {
// 	printf("============================\n");
// 	printf("TDES encrypt/decrypt single block\n");
// 
// 	// key
// 	muggle_des_key_block key1, key2, key3;
// 	key1.u64 = 0xf125cce7abe32f7c;
// 	key2.u64 = 0xf1ce7efc1eb52cab;
// 	key3.u64 = 0xefe77e7abe89fa7c;
// 	printf("key: \n");
// 	muggle_output_hex((unsigned char*)key1.bytes, 8, 0);
// 	muggle_output_hex((unsigned char*)key2.bytes, 8, 0);
// 	muggle_output_hex((unsigned char*)key3.bytes, 8, 0);
// 
// 	// generate subkey
// 	muggle_des_subkeys_t ks1, ks2, ks3;
// 	muggle_openssl_des_set_key_unchecked((unsigned char*)key1.bytes, &ks1);
// 	muggle_openssl_des_set_key_unchecked((unsigned char*)key2.bytes, &ks2);
// 	muggle_openssl_des_set_key_unchecked((unsigned char*)key3.bytes, &ks3);
// 
// 	// plaintext block
// 	muggle_64bit_block_t plaintext;
// 	for (int i = 0; i < 8; ++i)
// 	{
// 		plaintext.bytes[i] = 0x01 << i;
// 		// plaintext.bytes[i] = rand() % 256;
// 	}
// 	printf("input plaintext: ");
// 	muggle_output_hex(plaintext.bytes, 8, 0);
// 
// 	// encrypt
// 	muggle_64bit_block_t ciphertext;
// 	muggle_tdes_crypt(MUGGLE_ENCRYPT, &plaintext, &ciphertext, &ks1, &ks2, &ks3, NULL, NULL);
// 	printf("encryption ciphertext: ");
// 	muggle_output_hex(ciphertext.bytes, 8, 0);
// 
// 	// decrypt
// 	memset(&plaintext, 0, sizeof(plaintext));
// 	muggle_tdes_crypt(MUGGLE_DECRYPT, &ciphertext, &plaintext, &ks1, &ks2, &ks3, NULL, NULL);
// 	printf("decryption plaintext: ");
// 	muggle_output_hex(plaintext.bytes, 8, 0);
// }
// 
// void example_tdes_cipher(int mode)
// {
// 	printf("============================\n");
// 	printf("TDES encrypt/decrypt %s mode\n", block_cipher_mode_names[mode]);
// 
// 	// key
// 	muggle_des_key_block key1, key2, key3;
// 	key1.u64 = 0xf125cce7abe32f7c;
// 	key2.u64 = 0xf1ce7efc1eb52cab;
// 	key3.u64 = 0xefe77e7abe89fa7c;
// 	printf("key: \n");
// 	muggle_output_hex((unsigned char*)key1.bytes, 8, 0);
// 	muggle_output_hex((unsigned char*)key2.bytes, 8, 0);
// 	muggle_output_hex((unsigned char*)key3.bytes, 8, 0);
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
// 	if (muggle_tdes_cipher(mode, MUGGLE_ENCRYPT, input, output, space, key1, key2, key3, &iv, 1, NULL, NULL) != 0)
// 	{
// 		MUGGLE_LOG_ERROR("failed TDES encryption with %s mode", block_cipher_mode_names[mode]);
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
// 	if (muggle_tdes_cipher(mode, MUGGLE_DECRYPT, output, plaintext, space, key1, key2, key3, &iv, 1, NULL, NULL) != 0)
// 	{
// 		MUGGLE_LOG_ERROR("failed TDES decryption with %s mode", block_cipher_mode_names[mode]);
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
// 
// int main()
// {
// 	// simple init log
// 	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
// 	{
// 		MUGGLE_LOG_ERROR("failed initialize log");
// 		exit(EXIT_FAILURE);
// 	}
// 
// 	srand((unsigned int)time(NULL));
// 
// 	// crypt single block
// 	example_tdes_block();
// 
// 	// encrypt/decrypt plaintext
// 	example_tdes_cipher(MUGGLE_BLOCK_CIPHER_MODE_ECB);
// 	example_tdes_cipher(MUGGLE_BLOCK_CIPHER_MODE_CBC);
// 	example_tdes_cipher(MUGGLE_BLOCK_CIPHER_MODE_CFB);
// 	example_tdes_cipher(MUGGLE_BLOCK_CIPHER_MODE_OFB);
// 	example_tdes_cipher(MUGGLE_BLOCK_CIPHER_MODE_CTR);
// 
// 	return 0;
// }
