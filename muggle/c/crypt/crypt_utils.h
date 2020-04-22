/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_CRYPT_UTILS_H_
#define MUGGLE_C_CRYPT_UTILS_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>

EXTERN_C_BEGIN

enum
{
	MUGGLE_DECRYPT = 0, // decrypt
	MUGGLE_ENCRYPT, // encrypt
};

enum
{
	MUGGLE_BLOCK_CIPHER_MODE_ECB = 0,
	MUGGLE_BLOCK_CIPHER_MODE_CBC,
	MUGGLE_BLOCK_CIPHER_MODE_CFB,
	MUGGLE_BLOCK_CIPHER_MODE_OFB,
	MUGGLE_BLOCK_CIPHER_MODE_CTR,
	MAX_MUGGLE_BLOCK_CIPHER_MODE,
};

typedef union muggle_64bit_block
{
	unsigned char bytes[8];
	uint64_t u64;
	struct {
		uint32_t l;
		uint32_t h;
	}u32;
}muggle_64bit_block_t;

typedef union muggle_32bit_block
{
	unsigned char bytes[4];
	uint32_t u32;
	struct {
		uint16_t l;
		uint16_t h;
	}u16;
}muggle_32bit_block_t;

/*

   image a 8x8 matrix like Rubik's cube, rotate row and column's
   number are infinite loop

   rotate a 8x8 matrix row 1 forward 3 steps:
    0  1  2  3  4  5  6  7       0  1  2  3  4  5  6  7
    8  9 10 11 12 13 14 15      13 14 15  8  9 10 11 12
   16 17 18 19 20 21 22 23      16 17 18 19 20 21 22 23
   24 25 26 27 28 29 30 31  ->  24 25 26 27 28 29 30 31
   32 33 34 35 36 37 38 39      32 33 34 35 36 37 38 39
   40 41 42 43 44 45 46 47      40 41 42 43 44 45 46 47
   48 49 50 51 52 53 54 55      48 49 50 51 52 53 54 55
   56 57 58 59 60 61 62 63      56 57 58 59 60 61 62 63

   rotate a 8x8 matrix col 1 forward 3 steps:
    0  1  2  3  4  5  6  7       0 41  2  3  4  5  6  7
    8  9 10 11 12 13 14 15       8 49 10 11 12 13 14 15
   16 17 18 19 20 21 22 23      16 57 18 19 20 21 22 23
   24 25 26 27 28 29 30 31  ->  24  1 26 27 28 29 30 31
   32 33 34 35 36 37 38 39      32  9 34 35 36 37 38 39
   40 41 42 43 44 45 46 47      40 17 42 43 44 45 46 47
   48 49 50 51 52 53 54 55      48 25 50 51 52 53 54 55
   56 57 58 59 60 61 62 63      56 33 58 59 60 61 62 63

*/
MUGGLE_CC_EXPORT
void muggle_8x8bit_rotate_row(muggle_64bit_block_t *block, int row_idx, unsigned int step);

MUGGLE_CC_EXPORT
void muggle_8x8bit_rotate_col(muggle_64bit_block_t *block, int col_idx, unsigned int step);

// for debug
MUGGLE_CC_EXPORT
void muggle_output_hex(unsigned char *bytes, unsigned int len, int bytes_per_line);

MUGGLE_CC_EXPORT
void muggle_output_bin(unsigned char *bytes, unsigned int row, unsigned int col);

EXTERN_C_END

#endif
