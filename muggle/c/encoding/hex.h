/******************************************************************************
 *  @file         hex.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-21
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec hex
 *****************************************************************************/

#ifndef MUGGLE_C_HEX_H_
#define MUGGLE_C_HEX_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>

EXTERN_C_BEGIN

/**
 * @brief convert character in hex presentation to byte
 *
 * @param c  hex character
 *
 * @return byte
 */
MUGGLE_C_EXPORT
uint8_t muggle_hex_to_byte(char c);

/**
 * @brief convert hex representation string to bytes
 *
 * @param hex      input hex representation string
 * @param bytes    output bytes array
 * @param hex_len  length of hex
 *
 * @return
 *     - on success return 0
 *     - otherwise failed
 *
 * @note
 *     - user need gurantee the length of hex is an even number
 *     - user need gurantee the size of bytes at least 1/2 of hex
 */
MUGGLE_C_EXPORT
int muggle_hex_to_bytes(const char *hex, uint8_t *bytes, uint32_t hex_len);

/**
 * @brief convert bytes to hex representation string
 *
 * @param bytes      input bytes array
 * @param hex        output hex representation string
 * @param bytes_len  length of bytes
 *
 * @note
 *     - user need gurantee the size of hex at least 2 of bytes
 */
MUGGLE_C_EXPORT
void muggle_hex_from_bytes(const uint8_t *bytes, char *hex, uint32_t bytes_len);

EXTERN_C_END

#endif // !MUGGLE_C_HEX_H_
