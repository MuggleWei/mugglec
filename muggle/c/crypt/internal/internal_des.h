/******************************************************************************
 *  @file         internal_des.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec crpyt DES internal
 *****************************************************************************/
 
#ifndef MUGGLE_C_INTERNAL_DES_H_
#define MUGGLE_C_INTERNAL_DES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"
#include "muggle/c/crypt/des.h"

EXTERN_C_BEGIN

#define MUGGLE_CRYPT_DES_DEBUG 0

/**
 * @brief move single bit
 */
#define MUGGLE_MOVE_BIT(in, from, to) ((((in)>>(from))&0x01)<<(to))

/**
 * @brief key shift
 */
#define MUGGLE_DES_KEY_SHIFT(in, shift, mask) (((in)<<(shift))|(((in)>>(28-(shift)))&(mask)))

/**
 * @brief DES Initial Permutation
 *
 * @param in  input block
 * @param out output block
 */
void muggle_des_ip(const muggle_64bit_block_t *in, muggle_64bit_block_t *out);

/**
 * @brief DES Final Permutation(Inverse IP)
 *
 * @param in  input block
 * @param out output block
 */
void muggle_des_fp(const muggle_64bit_block_t *in, muggle_64bit_block_t *out);

/**
 * @brief DES Expand Permutation
 *
 * @param in   input block
 * @param out  48bits in 8 bytes
 */
void muggle_des_expand(const muggle_32bit_block_t *in, muggle_des_48bit_t *out);

/**
 * @brief DES S-Box
 *
 * @param in   input 48bits
 * @param out  4bit x 8 output
 */
void muggle_des_sbox(const muggle_des_subkey_t *in, muggle_des_48bit_t *out);

/**
 * @brief DES P permutation
 *
 * @param in   output of S-Box
 * @param out  output block
 */
void muggle_des_p(const muggle_32bit_block_t *in, muggle_32bit_block_t *out);

/**
 * @brief DES f function
 *
 * @param in  input block
 * @param out output block
 */
void muggle_des_f(const muggle_32bit_block_t *in, const muggle_des_subkey_t *sk, muggle_32bit_block_t *out);

/**
 * @brief DES PC-1
 *
 * @param in  input block
 * @param out output block
 */
void muggle_des_pc1(const muggle_64bit_block_t *in, muggle_64bit_block_t *out);

/**
 * @brief DES PC-2
 *
 * @param k   contain 28bit in k->u32.l and 28bit in k->u32.h
 * @param sk  output smuggle_des_subkey_t ub key
 */
void muggle_des_pc2(muggle_64bit_block_t *k, muggle_des_subkey_t *sk);

EXTERN_C_END

#endif
