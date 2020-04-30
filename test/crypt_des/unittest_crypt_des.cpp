#include <stdlib.h>
#include <string.h>
#include "gtest/gtest.h"
#if MUGGLE_TEST_LINK_OPENSSL
#include "openssl/des.h"
#endif

#include "muggle/c/muggle_c.h"
#include "muggle/c/crypt/internal/internal_des.h"

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

#define EXAMPLE_MESSAGE_LEN 64

void crypt_des_test(int block_cipher_mode)
{
	int ret;
	muggle_64bit_block_t key;
	muggle_64bit_block_t iv, iv2, iv_save;
	unsigned char input[EXAMPLE_MESSAGE_LEN], ciphertext[EXAMPLE_MESSAGE_LEN], plaintext[EXAMPLE_MESSAGE_LEN];
	unsigned int num_bytes = (unsigned int)sizeof(input);
	for (int i = 0; i < 1024; ++i)
	{
		gen_input_var(&key, &iv, input, num_bytes);
		iv_save.u64 = iv2.u64 = iv.u64;

		ret = muggle_des_cipher(block_cipher_mode, MUGGLE_ENCRYPT, key, input, num_bytes, &iv, 1, ciphertext);
		ASSERT_EQ(ret, 0);

		ret = muggle_des_cipher(block_cipher_mode, MUGGLE_DECRYPT, key, ciphertext, num_bytes, &iv2, 1, plaintext);
		ASSERT_EQ(ret, 0);

		ret = memcmp(input, plaintext, num_bytes);
		if (ret != 0)
		{
			printf("key: ");
			muggle_output_hex((unsigned char*)key.bytes, 8, 0);
			printf("iv: ");
			muggle_output_hex((unsigned char*)iv_save.bytes, 8, 0);
			printf("input: \n");
			muggle_output_hex((unsigned char*)input, 64, 8);
			printf("ciphertext: \n");
			muggle_output_hex((unsigned char*)ciphertext, 64, 8);
			printf("plaintext: \n");
			muggle_output_hex((unsigned char*)plaintext, 64, 8);
		}
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(iv.u64, iv2.u64);

#if MUGGLE_TEST_LINK_OPENSSL
		const_DES_cblock *openssl_key = (const_DES_cblock*)&key;
		DES_key_schedule openssl_ks;
		DES_set_key_unchecked(openssl_key, &openssl_ks);

		DES_cblock openssl_iv;
		memcpy(openssl_iv, iv_save.bytes, 8);
		unsigned char openssl_output[EXAMPLE_MESSAGE_LEN];

		int do_compare = 1;
		switch (block_cipher_mode)
		{
		case MUGGLE_BLOCK_CIPHER_MODE_CBC:
			{
				DES_ncbc_encrypt(input, openssl_output, num_bytes, &openssl_ks, &openssl_iv, DES_ENCRYPT);
			}break;
		case MUGGLE_BLOCK_CIPHER_MODE_CFB:
			{
				// TODO: numbit
				// DES_cfb_encrypt(input, openssl_output, 0, num_bytes, &openssl_ks, &openssl_iv, DES_ENCRYPT);
				do_compare = 0;
			}break;
		case MUGGLE_BLOCK_CIPHER_MODE_OFB:
			{
				// TODO: numbit
				// DES_ofb_encrypt(input, openssl_output, 0, num_bytes, &openssl_ks, &openssl_iv, DES_ENCRYPT);
				do_compare = 0;
			}break;
		case MUGGLE_BLOCK_CIPHER_MODE_CTR:
			{
				do_compare = 0;
			}break;
		default:
			{
				do_compare = 0;
			}
		}

		if (do_compare == 0)
		{
			continue;
		}

		ret = memcmp(ciphertext, openssl_output, num_bytes);
		if (ret != 0)
		{
			printf("muggle ciphertext: ");
			muggle_output_hex(ciphertext, 8, 16);
			printf("openssl ciphertext: ");
			muggle_output_hex(openssl_output, 8, 16);
		}
		ASSERT_EQ(ret, 0);

		ret = memcmp(iv.bytes, openssl_iv, 8);
		if (ret != 0)
		{
			printf("muggle iv: ");
			muggle_output_hex(iv.bytes, 8, 0);
			printf("openssl iv: ");
			muggle_output_hex(openssl_iv, 8, 0);
		}
		ASSERT_EQ(ret, 0);
#endif
	}
}

TEST(crypt_des, block_size)
{
	muggle_64bit_block_t block64;
	ASSERT_EQ(sizeof(block64), 8);

	muggle_32bit_block_t block32;
	ASSERT_EQ(sizeof(block32), 4);
}

TEST(crypt_des, IP)
{
	uint32_t ip_table[] = {
		58, 50, 42, 34, 26, 18, 10, 2,
		60, 52, 44, 36, 28, 20, 12, 4,
		62, 54, 46, 38, 30, 22, 14, 6,
		64, 56, 48, 40, 32, 24, 16, 8,
		57, 49, 41, 33, 25, 17, 9,  1,
		59, 51, 43, 35, 27, 19, 11, 3,
		61, 53, 45, 37, 29, 21, 13, 5,
		63, 55, 47, 39, 31, 23, 15, 7
	};
	for (uint32_t i = 0; i < sizeof(ip_table) / sizeof(ip_table[0]); i++)
	{
		muggle_64bit_block_t in;
		muggle_64bit_block_t out;

		// the bit need offset
		memset(&in, 0, sizeof(in));
		in.u64 = 0x01LLU << (ip_table[i]-1);
		muggle_des_ip(&in, &out);
		ASSERT_EQ(out.u64, 0x01LLU << i);
	}
}

TEST(crypt_des, FP)
{
	uint32_t fp_table[] = {
		40, 8, 48, 16, 56, 24, 64, 32,
		39, 7, 47, 15, 55, 23, 63, 31,
		38, 6, 46, 14, 54, 22, 62, 30,
		37, 5, 45, 13, 53, 21, 61, 29,
		36, 4, 44, 12, 52, 20, 60, 28,
		35, 3, 43, 11, 51, 19, 59, 27,
		34, 2, 42, 10, 50, 18, 58, 26,
		33, 1, 41, 9,  49, 17, 57, 25
	};
	for (uint32_t i = 0; i < sizeof(fp_table) / sizeof(fp_table[0]); i++)
	{
		muggle_64bit_block_t in;
		muggle_64bit_block_t out;

		// the bit need offset
		memset(&in, 0, sizeof(in));
		in.u64 = 0x01LLU << (fp_table[i]-1);
		muggle_des_fp(&in, &out);
		ASSERT_EQ(out.u64, 0x01LLU << i);
	}
}

TEST(crypt_des, IPFP)
{
	uint32_t ip_table[] = {
		58, 50, 42, 34, 26, 18, 10, 2,
		60, 52, 44, 36, 28, 20, 12, 4,
		62, 54, 46, 38, 30, 22, 14, 6,
		64, 56, 48, 40, 32, 24, 16, 8,
		57, 49, 41, 33, 25, 17, 9,  1,
		59, 51, 43, 35, 27, 19, 11, 3,
		61, 53, 45, 37, 29, 21, 13, 5,
		63, 55, 47, 39, 31, 23, 15, 7
	};

	muggle_64bit_block_t in, out1, out2;

	// the bit need offset
	for (uint32_t i = 0; i < sizeof(ip_table) / sizeof(ip_table[0]); i++)
	{
		uint32_t from = ip_table[i] - 1;
		memset(&in, 0, sizeof(in));
		in.u64 = 0x01LLU << from;
		muggle_des_ip(&in, &out1);
		muggle_des_fp(&out1, &out2);
		ASSERT_EQ(in.u64, out2.u64);
	}
}

TEST(crypt_des, E)
{
	uint32_t e_table[] = {
		32, 1,  2,  3,  4,  5,
		4,  5,  6,  7,  8,  9,
		8,  9,  10, 11, 12, 13,
		12, 13, 14, 15, 16, 17,
		16, 17, 18, 19, 20, 21,
		20, 21, 22, 23, 24, 25,
		24, 25, 26, 27, 28, 29,
		28, 29, 30, 31, 32, 1
	};

	muggle_32bit_block_t block;
	muggle_des_subkey_t out;
	for (uint32_t i = 0; i < 32; i++)
	{
		block.u32 = 0x01 << i;
		muggle_des_expand(&block, &out);
		// muggle_output_bin(out.bytes, 8, 6);

		for (uint32_t row = 0; row < 8; ++row)
		{
			for (uint32_t col = 0; col < 6; ++col)
			{
				uint32_t b = (out.bytes[row] >> col) & 0x01;
				uint32_t pos = row * 6 + col;
				if (e_table[pos] - 1 == i)
				{
					// printf("%u: [%u,%u]\n", i, row, col);
					ASSERT_EQ(b, 1);
				}
				else
				{
					ASSERT_EQ(b, 0);
				}
			}
		}
	}
}

TEST(crypt_des, sbox)
{
	unsigned char sbox_table[8][64] = {
		{
			// S1
			14, 4,  13, 1, 2,  15, 11, 8,  3,  10, 6,  12, 5,  9,  0, 7,
			0,  15, 7,  4, 14, 2,  13, 1,  10, 6,  12, 11, 9,  5,  3, 8,
			4,  1,  14, 8, 13, 6,  2,  11, 15, 12, 9,  7,  3,  10, 5, 0,
			15, 12, 8,  2, 4,  9,  1,  7,  5,  11, 3,  14, 10, 0,  6, 13
		},
		{
			// S2
			15, 1,  8,  14, 6,  11, 3,  4,  9,  7, 2,  13, 12, 0, 5,  10,
			3,  13, 4,  7,  15, 2,  8,  14, 12, 0, 1,  10, 6,  9, 11, 5,
			0,  14, 7,  11, 10, 4,  13, 1,  5,  8, 12, 6,  9,  3, 2,  15,
			13, 8,  10, 1,  3,  15, 4,  2,  11, 6, 7,  12, 0,  5, 14, 9
		},
		{
			// S3
			10, 0,  9,  14, 6, 3,  15, 5,  1,  13, 12, 7,  11, 4,  2,  8,
			13, 7,  0,  9,  3, 4,  6,  10, 2,  8,  5,  14, 12, 11, 15, 1,
			13, 6,  4,  9,  8, 15, 3,  0,  11, 1,  2,  12, 5,  10, 14, 7,
			1,  10, 13, 0,  6, 9,  8,  7,  4,  15, 14, 3,  11, 5,  2,  12
		},
		{
			// S4
			7,  13, 14, 3, 0,  6,  9,  10, 1,  2, 8, 5,  11, 12, 4,  15,
			13, 8,  11, 5, 6,  15, 0,  3,  4,  7, 2, 12, 1,  10, 14, 9,
			10, 6,  9,  0, 12, 11, 7,  13, 15, 1, 3, 14, 5,  2,  8,  4,
			3,  15, 0,  6, 10, 1,  13, 8,  9,  4, 5, 11, 12, 7,  2,  14
		},
		{
			// S5
			2,  12, 4,  1,  7,  10, 11, 6,  8,  5,  3,  15, 13, 0, 14, 9,
			14, 11, 2,  12, 4,  7,  13, 1,  5,  0,  15, 10, 3,  9, 8,  6,
			4,  2,  1,  11, 10, 13, 7,  8,  15, 9,  12, 5,  6,  3, 0,  14,
			11, 8,  12, 7,  1,  14, 2,  13, 6,  15, 0,  9,  10, 4, 5,  3
		},
		{
			// S6
			12, 1,  10, 15, 9, 2,  6,  8,  0,  13, 3,  4,  14, 7,  5,  11,
			10, 15, 4,  2,  7, 12, 9,  5,  6,  1,  13, 14, 0,  11, 3,  8,
			9,  14, 15, 5,  2, 8,  12, 3,  7,  0,  4,  10, 1,  13, 11, 6,
			4,  3,  2,  12, 9, 5,  15, 10, 11, 14, 1,  7,  6,  0,  8,  13
		},
		{
			// S7
			4,  11, 2,  14, 15, 0, 8,  13, 3,  12, 9, 7,  5,  10, 6, 1,
			13, 0,  11, 7,  4,  9, 1,  10, 14, 3,  5, 12, 2,  15, 8, 6,
			1,  4,  11, 13, 12, 3, 7,  14, 10, 15, 6, 8,  0,  5,  9, 2,
			6,  11, 13, 8,  1,  4, 10, 7,  9,  5,  0, 15, 14, 2,  3, 12
		},
		{
			// S8
			13, 2,  8,  4, 6,  15, 11, 1,  10, 9,  3,  14, 5,  0,  12, 7,
			1,  15, 13, 8, 10, 3,  7,  4,  12, 5,  6,  11, 0,  14, 9,  2,
			7,  11, 4,  1, 9,  12, 14, 2,  0,  6,  10, 13, 15, 3,  5,  8,
			2,  1,  14, 7, 4,  10, 8,  13, 15, 12, 9,  0,  3,  5,  6,  11
		}
	};

	int row_array[4] = {0x00, 0x01, 0x20, 0x21};
	int col_array[16];
	for (int i = 0; i < 16; i++)
	{
		col_array[i] = i;
	}

	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 16; col++)
		{
			int v = row_array[row] | (col_array[col] << 1);

			muggle_des_subkey_t p;
			memset(&p, 0, sizeof(p));
			for (int i = 0; i < 8; i++)
			{
				p.bytes[i] = v;
			}

			muggle_des_sbox(&p, &p);

			for (int i = 0; i < 8; i++)
			{
				ASSERT_EQ((uint32_t)p.bytes[i], sbox_table[i][row*16+col]);
			}
		}
	}
}

TEST(crypt_des, P)
{
	uint32_t p_table[] = {
		16, 7,  20, 21,
		29, 12, 28, 17,
		1,  15, 23, 26,
		5,  18, 31, 10,
		2,  8,  24, 14,
		32, 27, 3,  9,
		19, 13, 30, 6,
		22, 11, 4,  25
	};

	for (uint32_t i = 0; i < sizeof(p_table) / sizeof(p_table[0]); i++)
	{
		muggle_32bit_block_t block;

		memset(&block, 0, sizeof(block));
		block.u32 = 0x01 << (p_table[i] - 1);
		muggle_des_p(&block, &block);
		ASSERT_EQ(block.u32, 0x01 << i);
	}
}

TEST(crypt_des, EncrypDecrypt)
{
	unsigned char keys[][8] = {
		{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef},
		// {0x13, 0x34, 0x57, 0x79, 0x9b, 0xbc, 0xdf, 0xf1}
	};
	unsigned char plaintexts[][8] = {
		{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xE7},
		// {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
		// {0x54, 0x68, 0x65, 0x20, 0x71, 0x75, 0x66, 0x63}
	};

	int ret = 0;
	for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
	{
		for (size_t j = 0; j < sizeof(plaintexts) / sizeof(plaintexts[0]); j++)
		{
			muggle_64bit_block_t key;
			muggle_des_subkeys_t ks;
			muggle_64bit_block_t plaintext, ciphertext, decrypt_ret;

			// key
			memcpy(key.bytes, keys[i], 8);

			// plaintext
			memcpy(plaintext.bytes, plaintexts[j], 8);

			// encrypt genkey
			muggle_des_gen_subkeys(MUGGLE_ENCRYPT, &key, &ks);

			// encrypt
			muggle_des_crypt(&plaintext, &ks, &ciphertext);

			// decrypt genkey
			muggle_des_gen_subkeys(MUGGLE_DECRYPT, &key, &ks);

			// decrypt
			muggle_des_crypt(&ciphertext, &ks, &decrypt_ret);

			ret = memcmp(plaintext.bytes, decrypt_ret.bytes, 8);
			ASSERT_EQ(ret, 0);

#if MUGGLE_TEST_LINK_OPENSSL
			const_DES_cblock *openssl_input = &plaintexts[i];
			const_DES_cblock *openssl_key = &keys[i];
			DES_cblock openssl_output;
			DES_key_schedule openssl_ks;

			DES_set_key_unchecked(openssl_key, &openssl_ks);
			DES_ecb_encrypt(openssl_input, &openssl_output, &openssl_ks, DES_ENCRYPT);
			ret = memcmp(ciphertext.bytes, openssl_output, 8);
			if (ret != 0)
			{
				printf("muggle ciphertext: ");
				muggle_output_hex(ciphertext.bytes, 8, 0);
				printf("openssl ciphertext: ");
				muggle_output_hex(openssl_output, 8, 0);
			}
			ASSERT_EQ(ret, 0);
#endif
		}
	}
}

TEST(crypt_des, ecb)
{
	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_ECB);
}

TEST(crypt_des, cbc)
{
	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_CBC);
}

TEST(crypt_des, cfb)
{
	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_CFB);
}

TEST(crypt_des, ofb)
{
	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_OFB);
}

TEST(crypt_des, ctr)
{
	crypt_des_test(MUGGLE_BLOCK_CIPHER_MODE_CTR);
}
