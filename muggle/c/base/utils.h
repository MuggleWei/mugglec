/******************************************************************************
 *  @file         utils.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        function declaration of mugglec base utils
 *****************************************************************************/
 
#ifndef MUGGLE_UTILS_H_
#define MUGGLE_UTILS_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>

EXTERN_C_BEGIN

#define IS_POW_OF_2(x)             (!((x)&((x)-1)))
#define ROUND_UP_POW_OF_2_MUL(x,n) (((x)+(n)-1)&~((n)-1))

#define ALIGN_TRUE_SHARING(n) \
	(ROUND_UP_POW_OF_2_MUL(n, MUGGLE_CACHE_LINE_SIZE) \
	 + MUGGLE_CACHE_LINE_X2_SIZE)

#define MUGGLE_CHECK_RET(x, err_enum) \
	if (!(x)) \
	{ \
		return err_enum; \
	}

MUGGLE_C_EXPORT
uint64_t next_pow_of_2(uint64_t x);

EXTERN_C_END

#endif
