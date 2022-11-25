/******************************************************************************
 *  @file         internal_des.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec crypt DES internal
 *****************************************************************************/
 
#include "internal_des.h"
#include <stddef.h>
#include <string.h>
#include "muggle/c/base/utils.h"
#include "muggle/c/log/log.h"
#include "muggle/c/crypt/des.h"

static const unsigned char s_muggle_des_sbox_table[8][64] = {
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

void muggle_des_ip(const muggle_64bit_block_t *in, muggle_64bit_block_t *out)
{
	/*
	 * Initial Permutation table
	 *
	 *   58  50  42  34  26  18  10   2
	 *   60  52  44  36  28  20  12   4
	 *   62  54  46  38  30  22  14   6
	 *   64  56  48  40  32  24  16   8
	 *   57  49  41  33  25  17   9   1
	 *   59  51  43  35  27  19  11   3
	 *   61  53  45  37  29  21  13   5
	 *   63  55  47  39  31  23  15   7
	 *
	 * */
	memset(out, 0, sizeof(muggle_64bit_block_t));

	// move bit one by one
	out->u32.l = (uint32_t)(
		MUGGLE_MOVE_BIT(in->u64, 57, 0)  | MUGGLE_MOVE_BIT(in->u64, 49, 1)  | MUGGLE_MOVE_BIT(in->u64, 41, 2)  | MUGGLE_MOVE_BIT(in->u64, 33, 3)  |
		MUGGLE_MOVE_BIT(in->u64, 25, 4)  | MUGGLE_MOVE_BIT(in->u64, 17, 5)  | MUGGLE_MOVE_BIT(in->u64, 9, 6)   | MUGGLE_MOVE_BIT(in->u64, 1, 7)   |
		MUGGLE_MOVE_BIT(in->u64, 59, 8)  | MUGGLE_MOVE_BIT(in->u64, 51, 9)  | MUGGLE_MOVE_BIT(in->u64, 43, 10) | MUGGLE_MOVE_BIT(in->u64, 35, 11) |
		MUGGLE_MOVE_BIT(in->u64, 27, 12) | MUGGLE_MOVE_BIT(in->u64, 19, 13) | MUGGLE_MOVE_BIT(in->u64, 11, 14) | MUGGLE_MOVE_BIT(in->u64, 3, 15)  |
		MUGGLE_MOVE_BIT(in->u64, 61, 16) | MUGGLE_MOVE_BIT(in->u64, 53, 17) | MUGGLE_MOVE_BIT(in->u64, 45, 18) | MUGGLE_MOVE_BIT(in->u64, 37, 19) |
		MUGGLE_MOVE_BIT(in->u64, 29, 20) | MUGGLE_MOVE_BIT(in->u64, 21, 21) | MUGGLE_MOVE_BIT(in->u64, 13, 22) | MUGGLE_MOVE_BIT(in->u64, 5, 23)  |
		MUGGLE_MOVE_BIT(in->u64, 63, 24) | MUGGLE_MOVE_BIT(in->u64, 55, 25) | MUGGLE_MOVE_BIT(in->u64, 47, 26) | MUGGLE_MOVE_BIT(in->u64, 39, 27) |
		MUGGLE_MOVE_BIT(in->u64, 31, 28) | MUGGLE_MOVE_BIT(in->u64, 23, 29) | MUGGLE_MOVE_BIT(in->u64, 15, 30) | MUGGLE_MOVE_BIT(in->u64, 7, 31)
	);
	out->u32.h = (uint32_t)(
		MUGGLE_MOVE_BIT(in->u64, 56, 0)  | MUGGLE_MOVE_BIT(in->u64, 48, 1)  | MUGGLE_MOVE_BIT(in->u64, 40, 2)  | MUGGLE_MOVE_BIT(in->u64, 32, 3)  |
		MUGGLE_MOVE_BIT(in->u64, 24, 4)  | MUGGLE_MOVE_BIT(in->u64, 16, 5)  | MUGGLE_MOVE_BIT(in->u64, 8, 6)   | MUGGLE_MOVE_BIT(in->u64, 0, 7)   |
		MUGGLE_MOVE_BIT(in->u64, 58, 8)  | MUGGLE_MOVE_BIT(in->u64, 50, 9)  | MUGGLE_MOVE_BIT(in->u64, 42, 10) | MUGGLE_MOVE_BIT(in->u64, 34, 11) |
		MUGGLE_MOVE_BIT(in->u64, 26, 12) | MUGGLE_MOVE_BIT(in->u64, 18, 13) | MUGGLE_MOVE_BIT(in->u64, 10, 14) | MUGGLE_MOVE_BIT(in->u64, 2, 15)  |
		MUGGLE_MOVE_BIT(in->u64, 60, 16) | MUGGLE_MOVE_BIT(in->u64, 52, 17) | MUGGLE_MOVE_BIT(in->u64, 44, 18) | MUGGLE_MOVE_BIT(in->u64, 36, 19) |
		MUGGLE_MOVE_BIT(in->u64, 28, 20) | MUGGLE_MOVE_BIT(in->u64, 20, 21) | MUGGLE_MOVE_BIT(in->u64, 12, 22) | MUGGLE_MOVE_BIT(in->u64, 4, 23)  |
		MUGGLE_MOVE_BIT(in->u64, 62, 24) | MUGGLE_MOVE_BIT(in->u64, 54, 25) | MUGGLE_MOVE_BIT(in->u64, 46, 26) | MUGGLE_MOVE_BIT(in->u64, 38, 27) |
		MUGGLE_MOVE_BIT(in->u64, 30, 28) | MUGGLE_MOVE_BIT(in->u64, 22, 29) | MUGGLE_MOVE_BIT(in->u64, 14, 30) | MUGGLE_MOVE_BIT(in->u64, 6, 31)
	);
}

void muggle_des_fp(const muggle_64bit_block_t *in, muggle_64bit_block_t *out)
{
	/*
	 * Final Permutation(Inv Inital Permutation) table
	 *
	 *   40   8  48  16  56  24  64  32
	 *   39   7  47  15  55  23  63  31
	 *   38   6  46  14  54  22  62  30
	 *   37   5  45  13  53  21  61  29
	 *   36   4  44  12  52  20  60  28
	 *   35   3  43  11  51  19  59  27
	 *   34   2  42  10  50  18  58  26
	 *   33   1  41   9  49  17  57  25
	 *
	 * */
	memset(out, 0, sizeof(muggle_64bit_block_t));

	// move bit one by one
	out->u32.l = (uint32_t)(
		MUGGLE_MOVE_BIT(in->u64, 39, 0)  | MUGGLE_MOVE_BIT(in->u64, 7, 1)   | MUGGLE_MOVE_BIT(in->u64, 47, 2)  | MUGGLE_MOVE_BIT(in->u64, 15, 3)  |
		MUGGLE_MOVE_BIT(in->u64, 55, 4)  | MUGGLE_MOVE_BIT(in->u64, 23, 5)  | MUGGLE_MOVE_BIT(in->u64, 63, 6)  | MUGGLE_MOVE_BIT(in->u64, 31, 7)  |
		MUGGLE_MOVE_BIT(in->u64, 38, 8)  | MUGGLE_MOVE_BIT(in->u64, 6, 9)   | MUGGLE_MOVE_BIT(in->u64, 46, 10) | MUGGLE_MOVE_BIT(in->u64, 14, 11) |
		MUGGLE_MOVE_BIT(in->u64, 54, 12) | MUGGLE_MOVE_BIT(in->u64, 22, 13) | MUGGLE_MOVE_BIT(in->u64, 62, 14) | MUGGLE_MOVE_BIT(in->u64, 30, 15) |
		MUGGLE_MOVE_BIT(in->u64, 37, 16) | MUGGLE_MOVE_BIT(in->u64, 5, 17)  | MUGGLE_MOVE_BIT(in->u64, 45, 18) | MUGGLE_MOVE_BIT(in->u64, 13, 19) |
		MUGGLE_MOVE_BIT(in->u64, 53, 20) | MUGGLE_MOVE_BIT(in->u64, 21, 21) | MUGGLE_MOVE_BIT(in->u64, 61, 22) | MUGGLE_MOVE_BIT(in->u64, 29, 23) |
		MUGGLE_MOVE_BIT(in->u64, 36, 24) | MUGGLE_MOVE_BIT(in->u64, 4, 25)  | MUGGLE_MOVE_BIT(in->u64, 44, 26) | MUGGLE_MOVE_BIT(in->u64, 12, 27) |
		MUGGLE_MOVE_BIT(in->u64, 52, 28) | MUGGLE_MOVE_BIT(in->u64, 20, 29) | MUGGLE_MOVE_BIT(in->u64, 60, 30) | MUGGLE_MOVE_BIT(in->u64, 28, 31)
	);
	out->u32.h = (uint32_t)(
		MUGGLE_MOVE_BIT(in->u64, 35, 0)  | MUGGLE_MOVE_BIT(in->u64, 3, 1)   | MUGGLE_MOVE_BIT(in->u64, 43, 2)  | MUGGLE_MOVE_BIT(in->u64, 11, 3)  |
		MUGGLE_MOVE_BIT(in->u64, 51, 4)  | MUGGLE_MOVE_BIT(in->u64, 19, 5)  | MUGGLE_MOVE_BIT(in->u64, 59, 6)  | MUGGLE_MOVE_BIT(in->u64, 27, 7)  |
		MUGGLE_MOVE_BIT(in->u64, 34, 8)  | MUGGLE_MOVE_BIT(in->u64, 2, 9)   | MUGGLE_MOVE_BIT(in->u64, 42, 10) | MUGGLE_MOVE_BIT(in->u64, 10, 11) |
		MUGGLE_MOVE_BIT(in->u64, 50, 12) | MUGGLE_MOVE_BIT(in->u64, 18, 13) | MUGGLE_MOVE_BIT(in->u64, 58, 14) | MUGGLE_MOVE_BIT(in->u64, 26, 15) |
		MUGGLE_MOVE_BIT(in->u64, 33, 16) | MUGGLE_MOVE_BIT(in->u64, 1, 17)  | MUGGLE_MOVE_BIT(in->u64, 41, 18) | MUGGLE_MOVE_BIT(in->u64, 9, 19)  |
		MUGGLE_MOVE_BIT(in->u64, 49, 20) | MUGGLE_MOVE_BIT(in->u64, 17, 21) | MUGGLE_MOVE_BIT(in->u64, 57, 22) | MUGGLE_MOVE_BIT(in->u64, 25, 23) |
		MUGGLE_MOVE_BIT(in->u64, 32, 24) | MUGGLE_MOVE_BIT(in->u64, 0, 25)  | MUGGLE_MOVE_BIT(in->u64, 40, 26) | MUGGLE_MOVE_BIT(in->u64, 8, 27)  |
		MUGGLE_MOVE_BIT(in->u64, 48, 28) | MUGGLE_MOVE_BIT(in->u64, 16, 29) | MUGGLE_MOVE_BIT(in->u64, 56, 30) | MUGGLE_MOVE_BIT(in->u64, 24, 31)
	);
}

void muggle_des_expand(const muggle_32bit_block_t *in, muggle_des_48bit_t *out)
{
	/*
	 * Expand Permutation
	 *
	 *   32   1   2   3   4   5 
	 *    4   5   6   7   8   9
	 *    8   9  10  11  12  13
	 *   12  13  14  15  16  17
	 *   16  17  18  19  20  21
	 *   20  21  22  23  24  25
	 *   24  25  26  27  28  29
	 *   28  29  30  31  32   1
	 * */

	uint32_t u32 = in->u32;
	out->bytes[0] = (unsigned char)(((u32 >> 31) & 0x01) | (u32 << 1));
	out->bytes[1] = (unsigned char)(u32 >> 3);
	out->bytes[2] = (unsigned char)(u32 >> 7);
	out->bytes[3] = (unsigned char)(u32 >> 11);
	out->bytes[4] = (unsigned char)(u32 >> 15);
	out->bytes[5] = (unsigned char)(u32 >> 19);
	out->bytes[6] = (unsigned char)(u32 >> 23);
	out->bytes[7] = (unsigned char)(((u32 >> 27) & 0x1f) | (u32 << 5));
}

void muggle_des_sbox(const muggle_des_subkey_t *in, muggle_des_subkey_t *out)
{
	for (int i = 0; i < 8; i++)
	{
		unsigned char b = in->bytes[i];
		uint32_t idx = ((b & 0x020) | (b & 0x01) << 4) | ((b >> 1) & 0x0f);
		out->bytes[i] = s_muggle_des_sbox_table[i][idx];
	}
}

void muggle_des_p(const muggle_32bit_block_t *in, muggle_32bit_block_t *out)
{
	/*
	 * P Permutation
	 *   16   7  20  21  29  12  28  17
	 *    1  15  23  26   5  18  31  10
	 *    2   8  24  14  32  27   3   9
	 *   19  13  30   6  22  11   4  25
	 * */
	out->u32 = (uint32_t)(
		MUGGLE_MOVE_BIT(in->u32, 15, 0)   | MUGGLE_MOVE_BIT(in->u32, 6,  1)  | MUGGLE_MOVE_BIT(in->u32, 19, 2)  | MUGGLE_MOVE_BIT(in->u32, 20, 3)   |
		MUGGLE_MOVE_BIT(in->u32, 28, 4)   | MUGGLE_MOVE_BIT(in->u32, 11, 5)  | MUGGLE_MOVE_BIT(in->u32, 27, 6)  | MUGGLE_MOVE_BIT(in->u32, 16, 7)   |
		MUGGLE_MOVE_BIT(in->u32, 0,  8)   | MUGGLE_MOVE_BIT(in->u32, 14, 9)  | MUGGLE_MOVE_BIT(in->u32, 22, 10) | MUGGLE_MOVE_BIT(in->u32, 25, 11)  |
		MUGGLE_MOVE_BIT(in->u32, 4,  12)  | MUGGLE_MOVE_BIT(in->u32, 17, 13) | MUGGLE_MOVE_BIT(in->u32, 30, 14) | MUGGLE_MOVE_BIT(in->u32, 9,  15)  |
		MUGGLE_MOVE_BIT(in->u32, 1,  16)  | MUGGLE_MOVE_BIT(in->u32, 7, 17)  | MUGGLE_MOVE_BIT(in->u32, 23, 18) | MUGGLE_MOVE_BIT(in->u32, 13,  19) |
		MUGGLE_MOVE_BIT(in->u32, 31,  20) | MUGGLE_MOVE_BIT(in->u32, 26, 21) | MUGGLE_MOVE_BIT(in->u32, 2, 22)  | MUGGLE_MOVE_BIT(in->u32, 8,  23)  |
		MUGGLE_MOVE_BIT(in->u32, 18,  24) | MUGGLE_MOVE_BIT(in->u32, 12, 25) | MUGGLE_MOVE_BIT(in->u32, 29, 26) | MUGGLE_MOVE_BIT(in->u32, 5,  27)  |
		MUGGLE_MOVE_BIT(in->u32, 21,  28) | MUGGLE_MOVE_BIT(in->u32, 10, 29) | MUGGLE_MOVE_BIT(in->u32, 3, 30)  | MUGGLE_MOVE_BIT(in->u32, 24,  31)
	);
}

#if MUGGLE_CRYPT_DES_DEBUG
void convert_48bit_to_4bytes(const muggle_des_subkey_t *sk, unsigned char *bytes)
{
	bytes[0] = ((sk->bytes[0] & 0x3f) | ((sk->bytes[1] << 6) & 0xc0));
	bytes[1] = ((sk->bytes[1] >> 4) & 0x0f) | ((sk->bytes[2] << 4) & 0xf0);
	bytes[2] = ((sk->bytes[2] >> 4) & 0x03) | ((sk->bytes[3] << 2) & 0xfc);
	bytes[3] = ((sk->bytes[4] & 0x3f) | ((sk->bytes[5] << 6) & 0xc0));
	bytes[4] = ((sk->bytes[5] >> 4) & 0x0f) | ((sk->bytes[6] << 4) & 0xf0);
	bytes[5] = ((sk->bytes[6] >> 4) & 0x03) | ((sk->bytes[7] << 2) & 0xfc);
}
#endif

void muggle_des_f(const muggle_32bit_block_t *in, const muggle_des_subkey_t *sk, muggle_32bit_block_t *out)
{
	muggle_des_subkey_t p;

#if MUGGLE_CRYPT_DES_DEBUG
	printf("input: ");
	muggle_output_hex(in->bytes, 4, 0);
#endif

	// Expand Permutation
	muggle_des_expand(in, &p);

#if MUGGLE_CRYPT_DES_DEBUG
	unsigned char bytes[6];
	convert_48bit_to_4bytes(&p, bytes);
	printf("E: ");
	muggle_output_hex(bytes, 6, 0);
#endif

	// XOR subkey
	for (int i = 0; i < 8; i++)
	{
		p.bytes[i] = p.bytes[i] ^ sk->bytes[i];
	}

#if MUGGLE_CRYPT_DES_DEBUG
	convert_48bit_to_4bytes(&p, bytes);
	printf("XOR key: ");
	muggle_output_hex(bytes, 6, 0);
#endif

	// S-Box Permutation
	muggle_des_sbox(&p, &p);

#if MUGGLE_CRYPT_DES_DEBUG
	printf("sbox: ");
	muggle_output_hex(p.bytes, 8, 0);
#endif


	// const unsigned char *in_bytes = p.bytes;
	muggle_32bit_block_t u32_block;
	u32_block.bytes[0] = (p.bytes[0] & 0x0f) | ((p.bytes[1] << 4) & 0xf0);
	u32_block.bytes[1] = (p.bytes[2] & 0x0f) | ((p.bytes[3] << 4) & 0xf0);
	u32_block.bytes[2] = (p.bytes[4] & 0x0f) | ((p.bytes[5] << 4) & 0xf0);
	u32_block.bytes[3] = (p.bytes[6] & 0x0f) | ((p.bytes[7] << 4) & 0xf0);

	// P permutation
	muggle_des_p(&u32_block, out);
}

void muggle_des_pc1(const muggle_64bit_block_t *in, muggle_64bit_block_t *out)
{
	/*
	 * PC-1
	 *
	 * 57 49 41 33 25 17 9
	 * 1 58 50 42 34 26 18
	 * 10 2 59 51 43 35 27
	 * 19 11 3 60 52 44 36
	 * 
	 * 63 55 47 39 31 23 15
	 * 7 62 54 46 38 30 22
	 * 14 6 61 53 45 37 29
	 * 21 13 5 28 20 12 4
	 *
	 * */
	out->u32.l = (uint32_t)(
		MUGGLE_MOVE_BIT(in->u64, 56, 0)  | MUGGLE_MOVE_BIT(in->u64, 48, 1)  | MUGGLE_MOVE_BIT(in->u64, 40, 2)  | MUGGLE_MOVE_BIT(in->u64, 32, 3)  |
		MUGGLE_MOVE_BIT(in->u64, 24, 4)  | MUGGLE_MOVE_BIT(in->u64, 16, 5)  | MUGGLE_MOVE_BIT(in->u64, 8, 6)   | MUGGLE_MOVE_BIT(in->u64, 0, 7)   |
		MUGGLE_MOVE_BIT(in->u64, 57, 8)  | MUGGLE_MOVE_BIT(in->u64, 49, 9)  | MUGGLE_MOVE_BIT(in->u64, 41, 10) | MUGGLE_MOVE_BIT(in->u64, 33, 11) |
		MUGGLE_MOVE_BIT(in->u64, 25, 12) | MUGGLE_MOVE_BIT(in->u64, 17, 13) | MUGGLE_MOVE_BIT(in->u64, 9, 14)  | MUGGLE_MOVE_BIT(in->u64, 1, 15)  |
		MUGGLE_MOVE_BIT(in->u64, 58, 16) | MUGGLE_MOVE_BIT(in->u64, 50, 17) | MUGGLE_MOVE_BIT(in->u64, 42, 18) | MUGGLE_MOVE_BIT(in->u64, 34, 19) |
		MUGGLE_MOVE_BIT(in->u64, 26, 20) | MUGGLE_MOVE_BIT(in->u64, 18, 21) | MUGGLE_MOVE_BIT(in->u64, 10, 22) | MUGGLE_MOVE_BIT(in->u64, 2, 23)  |
		MUGGLE_MOVE_BIT(in->u64, 59, 24) | MUGGLE_MOVE_BIT(in->u64, 51, 25) | MUGGLE_MOVE_BIT(in->u64, 43, 26) | MUGGLE_MOVE_BIT(in->u64, 35, 27)
	);

	out->u32.h = (uint32_t)(
		MUGGLE_MOVE_BIT(in->u64, 62, 0)  | MUGGLE_MOVE_BIT(in->u64, 54, 1)  | MUGGLE_MOVE_BIT(in->u64, 46, 2)  | MUGGLE_MOVE_BIT(in->u64, 38, 3)  |
		MUGGLE_MOVE_BIT(in->u64, 30, 4)  | MUGGLE_MOVE_BIT(in->u64, 22, 5)  | MUGGLE_MOVE_BIT(in->u64, 14, 6)  | MUGGLE_MOVE_BIT(in->u64, 6, 7)   |
		MUGGLE_MOVE_BIT(in->u64, 61, 8)  | MUGGLE_MOVE_BIT(in->u64, 53, 9)  | MUGGLE_MOVE_BIT(in->u64, 45, 10) | MUGGLE_MOVE_BIT(in->u64, 37, 11) |
		MUGGLE_MOVE_BIT(in->u64, 29, 12) | MUGGLE_MOVE_BIT(in->u64, 21, 13) | MUGGLE_MOVE_BIT(in->u64, 13, 14) | MUGGLE_MOVE_BIT(in->u64, 5, 15)  |
		MUGGLE_MOVE_BIT(in->u64, 60, 16) | MUGGLE_MOVE_BIT(in->u64, 52, 17) | MUGGLE_MOVE_BIT(in->u64, 44, 18) | MUGGLE_MOVE_BIT(in->u64, 36, 19) |
		MUGGLE_MOVE_BIT(in->u64, 28, 20) | MUGGLE_MOVE_BIT(in->u64, 20, 21) | MUGGLE_MOVE_BIT(in->u64, 12, 22) | MUGGLE_MOVE_BIT(in->u64, 4, 23)  |
		MUGGLE_MOVE_BIT(in->u64, 27, 24) | MUGGLE_MOVE_BIT(in->u64, 19, 25) | MUGGLE_MOVE_BIT(in->u64, 11, 26) | MUGGLE_MOVE_BIT(in->u64, 3, 27)
	);
}

void muggle_des_pc2(muggle_64bit_block_t *k, muggle_des_subkey_t *sk)
{
	/*
	 * PC-2
	 *
	 * 14 17 11 24  1  5
	 *  3 28 15  6 21 10
	 * 23 19 12  4 26  8
	 * 16  7 27 20 13  2
	 * 41 52 31 37 47 55
	 * 30 40 51 45 33 48
	 * 44 49 39 56 34 53
	 * 46 42 50 36 29 32
	 * */


	uint32_t c = k->u32.l;
	uint32_t d = k->u32.h;
	sk->bytes[0] = MUGGLE_MOVE_BIT(c, 13, 0) | MUGGLE_MOVE_BIT(c, 16, 1) | MUGGLE_MOVE_BIT(c, 10, 2) | MUGGLE_MOVE_BIT(c, 23, 3) | MUGGLE_MOVE_BIT(c, 0, 4)  | MUGGLE_MOVE_BIT(c, 4, 5);
	sk->bytes[1] = MUGGLE_MOVE_BIT(c, 2, 0)  | MUGGLE_MOVE_BIT(c, 27, 1) | MUGGLE_MOVE_BIT(c, 14, 2) | MUGGLE_MOVE_BIT(c, 5, 3)  | MUGGLE_MOVE_BIT(c, 20, 4) | MUGGLE_MOVE_BIT(c, 9, 5);
	sk->bytes[2] = MUGGLE_MOVE_BIT(c, 22, 0) | MUGGLE_MOVE_BIT(c, 18, 1) | MUGGLE_MOVE_BIT(c, 11, 2) | MUGGLE_MOVE_BIT(c, 3, 3)  | MUGGLE_MOVE_BIT(c, 25, 4) | MUGGLE_MOVE_BIT(c, 7, 5);
	sk->bytes[3] = MUGGLE_MOVE_BIT(c, 15, 0) | MUGGLE_MOVE_BIT(c, 6, 1)  | MUGGLE_MOVE_BIT(c, 26, 2) | MUGGLE_MOVE_BIT(c, 19, 3) | MUGGLE_MOVE_BIT(c, 12, 4) | MUGGLE_MOVE_BIT(c, 1, 5);
	sk->bytes[4] = MUGGLE_MOVE_BIT(d, 12, 0) | MUGGLE_MOVE_BIT(d, 23, 1) | MUGGLE_MOVE_BIT(d, 2, 2)  | MUGGLE_MOVE_BIT(d, 8, 3)  | MUGGLE_MOVE_BIT(d, 18, 4) | MUGGLE_MOVE_BIT(d, 26, 4);
	sk->bytes[5] = MUGGLE_MOVE_BIT(d, 1, 0)  | MUGGLE_MOVE_BIT(d, 11, 1) | MUGGLE_MOVE_BIT(d, 22, 2) | MUGGLE_MOVE_BIT(d, 16, 3) | MUGGLE_MOVE_BIT(d, 4, 4)  | MUGGLE_MOVE_BIT(d, 19, 4);
	sk->bytes[6] = MUGGLE_MOVE_BIT(d, 15, 0) | MUGGLE_MOVE_BIT(d, 20, 1) | MUGGLE_MOVE_BIT(d, 10, 2) | MUGGLE_MOVE_BIT(d, 27, 3) | MUGGLE_MOVE_BIT(d, 5, 4)  | MUGGLE_MOVE_BIT(d, 24, 4);
	sk->bytes[7] = MUGGLE_MOVE_BIT(d, 17, 0) | MUGGLE_MOVE_BIT(d, 13, 1) | MUGGLE_MOVE_BIT(d, 21, 2) | MUGGLE_MOVE_BIT(d, 7, 3)  | MUGGLE_MOVE_BIT(d, 0, 4)  | MUGGLE_MOVE_BIT(d, 3, 4);
}

