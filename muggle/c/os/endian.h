/******************************************************************************
 *  @file         endian.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-09-07
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec endian
 *****************************************************************************/

#ifndef MUGGLE_C_ENDIAN_H_
#define MUGGLE_C_ENDIAN_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>

EXTERN_C_BEGIN

#define MUGGLE_ENDIAN_SWAP_16(value) \
	(((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8))

#define MUGGLE_ENDIAN_SWAP_32(value) \
	(((value & 0x000000FF) << 24) | \
	 ((value & 0x0000FF00) <<  8) | \
	 ((value & 0x00FF0000) >>  8) | \
	 ((value & 0xFF000000) >> 24))

#define MUGGLE_ENDIAN_SWAP_64(value) \
	(((value & 0x00000000000000FF) << 56) | \
	 ((value & 0x000000000000FF00) << 40) | \
	 ((value & 0x0000000000FF0000) << 24) | \
	 ((value & 0x00000000FF000000) <<  8) | \
	 ((value & 0x000000FF00000000) >>  8) | \
	 ((value & 0x0000FF0000000000) >> 24) | \
	 ((value & 0x00FF000000000000) >> 40) | \
	 ((value & 0xFF00000000000000) >> 56))

#define MUGGLE_LITTLE_ENDIAN 0 //!< little endian
#define MUGGLE_BIG_ENDIAN    1 //!< big endian

/**
 * @brief get endianness
 *
 * @return MUGGLE_LITTLE_ENDIAN or MUGGLE_BIG_ENDIAN
 */
MUGGLE_C_EXPORT
int muggle_endianness();

EXTERN_C_END

#endif /* ifndef MUGGLE_C_ENDIAN_H_ */
