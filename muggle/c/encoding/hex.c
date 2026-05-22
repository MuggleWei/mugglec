/******************************************************************************
 *  @file         hex.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-21
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec hex
 *****************************************************************************/

#include "hex.h"


uint8_t muggle_hex_to_byte(char c)
{
	if ('0' <= c && c <= '9') return (uint8_t)(c - '0');
    if ('A' <= c && c <= 'F') return (uint8_t)(c - 'A' + 10);
    if ('a' <= c && c <= 'f') return (uint8_t)(c - 'a' + 10);
    return (uint8_t)(-1);
}

int muggle_hex_to_bytes(const char *hex, uint8_t *bytes, uint32_t hex_len)
{
	for (uint32_t i = 0; i < hex_len / 2; i++)
	{
		uint8_t h = muggle_hex_to_byte(hex[i*2 + 0]);
		uint8_t l = muggle_hex_to_byte(hex[i*2 + 1]);
		if (h == (uint8_t)(-1) || l == (uint8_t)(-1))
		{
			return -1;
		}
		bytes[i] = h << 4 | l;
	}

	return 0;
}

void muggle_hex_from_bytes(const uint8_t *bytes, char *hex, uint32_t bytes_len)
{
	static const char s_hex[] = "0123456789ABCDEF";

	for (uint32_t i = 0; i < bytes_len; i++)
	{
		hex[i*2 + 0] = s_hex[(bytes[i] >> 4) & 0x0f];
		hex[i*2 + 1] = s_hex[bytes[i] & 0x0f];
	}
}
