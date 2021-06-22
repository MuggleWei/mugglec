/******************************************************************************
 *  @file         internal_aes.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec crypt AES internal 
 *****************************************************************************/
 
#ifndef MUGGLE_C_INTERNAL_AES_H_
#define MUGGLE_C_INTERNAL_AES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"

EXTERN_C_BEGIN

#define MUGGLE_CRYPT_AES_DEBUG 0

struct muggle_aes_sub_keys;


/**
 * @brief AES Add Round Key
 */
void muggle_aes_add_round_key(unsigned char *state, const uint32_t *rd_key);

/**
 * @brief AES SubBytes Transformation
 *
 * @param state a rectangular array of 16 bytes
 */
void muggle_aes_sub_bytes(unsigned char *state);

/**
 * @brief AES InvSubBytes Transformation
 *
 * @param state a rectangular array of 16 bytes
 */
void muggle_aes_inv_sub_bytes(unsigned char *state);

/**
 * @brief AES Shift Rows
 *
 * @param state a rectangular array of 16 bytes
 */
void muggle_aes_shift_rows(unsigned char *state);

/**
 * @brief AES Inv Shift Rows
 *
 * @param state a rectangular array of 16 bytes
 */
void muggle_aes_inv_shift_rows(unsigned char *state);

/**
 * @brief AES Mix Column
 *
 * @param state a rectangular array of 16 bytes
 */
void muggle_aes_mix_column(unsigned char *state);

/**
 * @brief AES Inv Mix Column
 *
 * @param state a rectangular array of 16 bytes
 */
void muggle_aes_inv_mix_column(unsigned char *state);

/**
 * @brief AES RotWord
 */
uint32_t muggle_aes_rot_word(uint32_t word);

/**
 * @brief AES SubWord
 */
uint32_t muggle_aes_sub_word(uint32_t word);

/**
 * @brief AES encrypt
 */
int muggle_aes_encrypt(
	unsigned char *state,
	const struct muggle_aes_sub_keys *sk);

/**
 * @brief AES decrypt
 */
int muggle_aes_decrypt(
	unsigned char *state,
	const struct muggle_aes_sub_keys *sk);


EXTERN_C_END

#endif
