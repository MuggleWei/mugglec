/******************************************************************************
 *  @file         internal_aes.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec crypt AES internal
 *****************************************************************************/
 
#include "internal_aes.h"
#include <string.h>
#include "muggle/c/crypt/aes.h"
#include "muggle/c/base/err.h"
#include "muggle/c/log/log.h"

static const unsigned char s_muggle_aes_sbox[256] = {
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const unsigned char s_muggle_aes_inv_sbox[256] = {
	0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
	0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
	0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
	0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
	0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
	0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
	0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
	0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
	0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
	0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
	0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
	0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
	0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
	0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
	0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
	0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

static const unsigned char s_muggle_aes_mix_column[16] = {
	0x02, 0x03, 0x01, 0x01,
	0x01, 0x02, 0x03, 0x01,
	0x01, 0x01, 0x02, 0x03,
	0x03, 0x01, 0x01, 0x02
};

static const unsigned char s_muggle_aes_inv_mix_column[16] = {
	0x0e, 0x0b, 0x0d, 0x09,
	0x09, 0x0e, 0x0b, 0x0d,
	0x0d, 0x09, 0x0e, 0x0b,
	0x0b, 0x0d, 0x09, 0x0e,
};

// static const uint32_t s_aes_rcon[] = {
//     0x01000000,0x02000000,0x04000000,0x08000000,
//     0x10000000,0x20000000,0x40000000,0x80000000,
//     0x1b000000,0x36000000,0x6c000000,0xd8000000,
//     0xab000000,0x4d000000,0x9a000000
// };

// incorrect result return from this function
// /* 
//  * From: https://en.wikipedia.org/wiki/Finite_field_arithmetic#Rijndael's_(AES)_finite_field
//  * Multiply two numbers in the GF(2^8) finite field defined 
//  * by the polynomial x^8 + x^4 + x^3 + x + 1 = 0
//  * using the Russian Peasant Multiplication algorithm
//  * (the other way being to do carry-less multiplication followed by a modular reduction)
//  *
//  * NOTE: 
//  * This example has cache, timing, and branch prediction side-channel leaks, and is not suitable for use in cryptography.
//  * This function only use for explain theory, the real project will use another part codes that generate project 
//  * with option MUGGLE_CRYPT_OPTIMIZATION 
//  *  
//  * */
// static unsigned char galois_mul(unsigned char a, unsigned char b)
// {
// 	unsigned char p = 0;
// 	while (a & b)
// 	{
// 		if (b & 1) // if b is odd, then add the corresponding a to p (final product = sum of all a's corresponding to odd b's)
// 		{
// 			p ^= a; // since we're in GF(2^m), addition is an XOR
// 		}
// 
// 		if (a & 0x80) // GF modulo: if a >= 128, then it will overflow when shifted left, so reduce
// 		{
// 			// XOR with the primitive polynomial x^8 + x^4 + x^3 + x + 1 (0b1_0001_1011) - you can change it but it must be irreducible
// 			a = (a << 1) ^ 0x11b;
// 		}
// 		else
// 		{
// 			a <<= 1; // equivalent to a*2
// 		}
// 		b >>= 1; // equivalent to b // 2
// 	}
// 
// 	return p;
// }

static unsigned char galois_mul(unsigned char a, unsigned char b)
{
	unsigned char r = 0, t = 0;
	while (a != 0)
	{
		if ((a & 0x01) != 0)
		{
			r = r ^ b;
		}
		t = b & 0x80;
		b = b << 1;
		if (t != 0)
		{
			b = b ^ 0x1b;
		}
		a = a >> 1;
	}
	return r;
}

void muggle_aes_add_round_key(unsigned char *state, const uint32_t *rd_key)
{
	state[0] ^= ((rd_key[0] >> 24) & 0xff);
	state[4] ^= ((rd_key[0] >> 16) & 0xff);
	state[8] ^= ((rd_key[0] >> 8) & 0xff);
	state[12] ^= ((rd_key[0]) & 0xff);

	state[1] ^= ((rd_key[1] >> 24) & 0xff);
	state[5] ^= ((rd_key[1] >> 16) & 0xff);
	state[9] ^= ((rd_key[1] >> 8) & 0xff);
	state[13] ^= ((rd_key[1]) & 0xff);

	state[2] ^= ((rd_key[2] >> 24) & 0xff);
	state[6] ^= ((rd_key[2] >> 16) & 0xff);
	state[10] ^= ((rd_key[2] >> 8) & 0xff);
	state[14] ^= ((rd_key[2]) & 0xff);

	state[3] ^= ((rd_key[3] >> 24) & 0xff);
	state[7] ^= ((rd_key[3] >> 16) & 0xff);
	state[11] ^= ((rd_key[3] >> 8) & 0xff);
	state[15] ^= ((rd_key[3]) & 0xff);
}

void muggle_aes_sub_bytes(unsigned char *state)
{
	for (int i = 0; i < 16; i++)
	{
		state[i] = s_muggle_aes_sbox[state[i]];
	}
}

void muggle_aes_inv_sub_bytes(unsigned char *state)
{
	for (int i = 0; i < 16; i++)
	{
		state[i] = s_muggle_aes_inv_sbox[state[i]];
	}
}

void muggle_aes_shift_rows(unsigned char *state)
{
	unsigned char t;

	// row 0 don't move
	// row 1 shift left by 1
	t = state[4];
	state[4] = state[5];
	state[5] = state[6];
	state[6] = state[7];
	state[7] = t;

	// row 2 shift left by 2
	t = state[8];
	state[8] = state[10];
	state[10] = t;
	t = state[9];
	state[9] = state[11];
	state[11] = t;

	// row 3 shift left by 3(shift right by 1)
	t = state[15];
	state[15] = state[14];
	state[14] = state[13];
	state[13] = state[12];
	state[12] = t;
}

void muggle_aes_inv_shift_rows(unsigned char *state)
{
	unsigned char t;

	// row 1 shift right by 1
	t = state[7];
	state[7] = state[6];
	state[6] = state[5];
	state[5] = state[4];
	state[4] = t;

	// row 2 shift right by 2
	t = state[8];
	state[8] = state[10];
	state[10] = t;
	t = state[9];
	state[9] = state[11];
	state[11] = t;

	// row 3 shift right by 3
	t = state[12];
	state[12] = state[13];
	state[13] = state[14];
	state[14] = state[15];
	state[15] = t;
}

void muggle_aes_mix_column(unsigned char *state)
{
	unsigned char arr[16];

	memcpy(arr, state, 16);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			int idx = (i << 2) | j;
			state[idx] =
				galois_mul(s_muggle_aes_mix_column[i*4 + 0], arr[0*4 + j]) ^
				galois_mul(s_muggle_aes_mix_column[i*4 + 1], arr[1*4 + j]) ^
				galois_mul(s_muggle_aes_mix_column[i*4 + 2], arr[2*4 + j]) ^
				galois_mul(s_muggle_aes_mix_column[i*4 + 3], arr[3*4 + j]);
		}
	}
}

void muggle_aes_inv_mix_column(unsigned char *state)
{
	unsigned char arr[16];

	memcpy(arr, state, 16);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			int idx = (i << 2) | j;
			state[idx] =
				galois_mul(s_muggle_aes_inv_mix_column[i*4 + 0], arr[0*4 + j]) ^
				galois_mul(s_muggle_aes_inv_mix_column[i*4 + 1], arr[1*4 + j]) ^
				galois_mul(s_muggle_aes_inv_mix_column[i*4 + 2], arr[2*4 + j]) ^
				galois_mul(s_muggle_aes_inv_mix_column[i*4 + 3], arr[3*4 + j]);
		}
	}
}

uint32_t muggle_aes_rot_word(uint32_t word)
{
	uint32_t ret = 0;
	unsigned char *p_ret = (unsigned char*)&ret;
	unsigned char *p_word = (unsigned char*)&word;
	p_ret[0] = p_word[3];
	p_ret[1] = p_word[0];
	p_ret[2] = p_word[1];
	p_ret[3] = p_word[2];
	return ret;
}

uint32_t muggle_aes_sub_word(uint32_t word)
{
    uint32_t result;

	unsigned char *p_ret = (unsigned char*)&result;
	unsigned char *p_word = (unsigned char*)&word;
	p_ret[0] = s_muggle_aes_sbox[p_word[0]];
	p_ret[1] = s_muggle_aes_sbox[p_word[1]];
	p_ret[2] = s_muggle_aes_sbox[p_word[2]];
	p_ret[3] = s_muggle_aes_sbox[p_word[3]];

    return result;
}

int muggle_aes_encrypt(
	unsigned char *state,
	const muggle_aes_subkeys_t *sk)
{
	if (sk->rounds != 10 && sk->rounds != 12 && sk->rounds != 14)
	{
		MUGGLE_ASSERT_MSG(
			(sk->rounds == 10 || sk->rounds == 12 || sk->rounds == 14),
			"Invalid AES key schedule rounds"
		);
		return MUGGLE_ERR_INVALID_PARAM;
	}

#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[0]: Start of Round\n");
	muggle_output_hex(state, 16, 4);
	printf("AES encrypt[0]: Round Key Value\n");
	muggle_output_hex((unsigned char*)&sk->rd_key[0], 16, 4);
#endif

	muggle_aes_add_round_key(state, &sk->rd_key[0]);

	// round 1
#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[1]: Start of Round\n");
	muggle_output_hex(state, 16, 4);
#endif

	muggle_aes_sub_bytes(state);

#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[1]: After SubBytes\n");
	muggle_output_hex(state, 16, 4);
#endif

	muggle_aes_shift_rows(state);

#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[1]: After ShiftRows\n");
	muggle_output_hex(state, 16, 4);
#endif

	muggle_aes_mix_column(state);

#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[1]: After MixColumns\n");
	muggle_output_hex(state, 16, 4);
#endif

	muggle_aes_add_round_key(state, &sk->rd_key[4]);

#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[1]: Round Key Value\n");
	muggle_output_hex((unsigned char*)&sk->rd_key[4], 16, 4);
#endif

	// round 2 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[8]);

	// round 3 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[12]);

	// round 4 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[16]);

	// round 5 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[20]);

	// round 6 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[24]);

	// round 7 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[28]);

	// round 8 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[32]);

	// round 9 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[36]);

	if (sk->rounds == 10)
	{
		// round 10
#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES encrypt[10]: Start of Round\n");
		muggle_output_hex(state, 16, 4);
#endif

		muggle_aes_sub_bytes(state);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES encrypt[10]: After SubBytes\n");
		muggle_output_hex(state, 16, 4);
#endif

		muggle_aes_shift_rows(state);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES encrypt[10]: After ShiftRows\n");
		muggle_output_hex(state, 16, 4);
		printf("AES encrypt[10]: Round Key Value\n");
		muggle_output_hex((unsigned char*)&sk->rd_key[40], 16, 4);
#endif

		muggle_aes_add_round_key(state, &sk->rd_key[40]);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES encrypt[10]: After AddRoundKey\n");
		muggle_output_hex(state, 16, 4);
#endif
	}
	else
	{
		// round 10
		muggle_aes_sub_bytes(state);
		muggle_aes_shift_rows(state);
		muggle_aes_mix_column(state);
		muggle_aes_add_round_key(state, &sk->rd_key[40]);

		// round 11
		muggle_aes_sub_bytes(state);
		muggle_aes_shift_rows(state);
		muggle_aes_mix_column(state);
		muggle_aes_add_round_key(state, &sk->rd_key[44]);

		if (sk->rounds == 12)
		{
			// round 12
			muggle_aes_sub_bytes(state);
			muggle_aes_shift_rows(state);
			muggle_aes_add_round_key(state, &sk->rd_key[48]);
		}
		else if (sk->rounds == 14)
		{
			// round 12
			muggle_aes_sub_bytes(state);
			muggle_aes_shift_rows(state);
			muggle_aes_mix_column(state);
			muggle_aes_add_round_key(state, &sk->rd_key[48]);

			// round 13
			muggle_aes_sub_bytes(state);
			muggle_aes_shift_rows(state);
			muggle_aes_mix_column(state);
			muggle_aes_add_round_key(state, &sk->rd_key[52]);

			// round 14
			muggle_aes_sub_bytes(state);
			muggle_aes_shift_rows(state);
			muggle_aes_add_round_key(state, &sk->rd_key[56]);
		}
	}

	return 0;
}

int muggle_aes_decrypt(
	unsigned char *state,
	const muggle_aes_subkeys_t *sk)
{
	if (sk->rounds != 10 && sk->rounds != 12 && sk->rounds != 14)
	{
		MUGGLE_ASSERT_MSG(
			(sk->rounds == 10 || sk->rounds == 12 || sk->rounds == 14),
			"Invalid AES key schedule rounds"
		);
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (sk->rounds > 10)
	{
		if (sk->rounds == 14)
		{
			// inv round 14
			muggle_aes_add_round_key(state, &sk->rd_key[56]);
			muggle_aes_inv_shift_rows(state);
			muggle_aes_inv_sub_bytes(state);

			// inv round 13
			muggle_aes_add_round_key(state, &sk->rd_key[52]);
			muggle_aes_inv_mix_column(state);
			muggle_aes_inv_shift_rows(state);
			muggle_aes_inv_sub_bytes(state);

			// inv round 12
			muggle_aes_add_round_key(state, &sk->rd_key[48]);
			muggle_aes_inv_mix_column(state);
			muggle_aes_inv_shift_rows(state);
			muggle_aes_inv_sub_bytes(state);
		}
		else
		{
			// inv round 12
			muggle_aes_add_round_key(state, &sk->rd_key[48]);
			muggle_aes_inv_shift_rows(state);
			muggle_aes_inv_sub_bytes(state);
		}

		// inv round 11
		muggle_aes_add_round_key(state, &sk->rd_key[44]);
		muggle_aes_inv_mix_column(state);
		muggle_aes_inv_shift_rows(state);
		muggle_aes_inv_sub_bytes(state);

		// inv round 10
		muggle_aes_add_round_key(state, &sk->rd_key[40]);
		muggle_aes_inv_mix_column(state);
		muggle_aes_inv_shift_rows(state);
		muggle_aes_inv_sub_bytes(state);
	}
	else
	{
		// inv round 10
#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES decrypt[inv 10]: Start of Round\n");
		muggle_output_hex(state, 16, 4);
		printf("AES decrypt[inv 10]: Round Key Value\n");
		muggle_output_hex((unsigned char*)&sk->rd_key[40], 16, 4);
#endif

		muggle_aes_add_round_key(state, &sk->rd_key[40]);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES decrypt[inv 10]: After AddRoundKey\n");
		muggle_output_hex(state, 16, 4);
#endif

		muggle_aes_inv_shift_rows(state);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES decrypt[inv 10]: After ShiftRows\n");
		muggle_output_hex(state, 16, 4);
#endif

		muggle_aes_inv_sub_bytes(state);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES encrypt[inv 10]: After SubBytes\n");
		muggle_output_hex(state, 16, 4);
#endif
	}

	// inv round 9
	muggle_aes_add_round_key(state, &sk->rd_key[36]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 8
	muggle_aes_add_round_key(state, &sk->rd_key[32]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 7
	muggle_aes_add_round_key(state, &sk->rd_key[28]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 6
	muggle_aes_add_round_key(state, &sk->rd_key[24]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 5
	muggle_aes_add_round_key(state, &sk->rd_key[20]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 4
	muggle_aes_add_round_key(state, &sk->rd_key[16]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 3
	muggle_aes_add_round_key(state, &sk->rd_key[12]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 2
	muggle_aes_add_round_key(state, &sk->rd_key[8]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 1
	muggle_aes_add_round_key(state, &sk->rd_key[4]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 0
	muggle_aes_add_round_key(state, &sk->rd_key[0]);

	return 0;
}

