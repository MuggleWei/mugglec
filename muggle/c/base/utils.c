/******************************************************************************
 *  @file         utils.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec base utils
 *****************************************************************************/
 
#include "muggle/c/base/utils.h"

// from https://github.com/cloudwu/buddy/blob/master/buddy.c
uint64_t next_pow_of_2(uint64_t x)
{
	if (IS_POW_OF_2(x))
		return x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}
