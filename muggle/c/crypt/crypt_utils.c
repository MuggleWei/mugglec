/******************************************************************************
 *  @file         crypt_utils.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec crypt utils
 *****************************************************************************/
 
#include "crypt_utils.h"
#include <stdio.h>

#define MUGGLE_8X8BITS_COL_MASK 0x0101010101010101L

void muggle_8x8bit_rotate_row(muggle_64bit_block_t *block, int row_idx, unsigned int step)
{
	unsigned char c = block->bytes[row_idx];
	block->bytes[row_idx] = (c << step) | (c >> (8 - step));
}
void muggle_8x8bit_rotate_col(muggle_64bit_block_t *block, int col_idx, unsigned int step)
{
	step *= 8;
	uint64_t mask = MUGGLE_8X8BITS_COL_MASK << col_idx;
	uint64_t v = block->u64 & mask;
	block->u64 = ((v << step) | (v >> (64 - step))) | (block->u64 & (~mask));
}

void muggle_output_hex(const unsigned char *bytes, unsigned int len, int bytes_per_line)
{
	int cnt = 0;
	for (unsigned int i = 0; i < len; ++i)
	{
		printf("%02x ", 0xff & bytes[i]);
		if (bytes_per_line > 0 && ++cnt == bytes_per_line)
		{
			cnt = 0;
			printf("\n");
		}
	}
	printf("\n");
}
void muggle_output_bin(const unsigned char *bytes, unsigned int row, unsigned int col)
{
	unsigned int val = 0;
	for (unsigned int i = 0; i < row; ++i)
	{
		for (unsigned int j = 0; j < col; ++j)
		{
			val = (0x01<<j) & bytes[i];
			printf("%u ", val == 0 ? 0 : 1);
		}
		printf("\n");
	}
}
