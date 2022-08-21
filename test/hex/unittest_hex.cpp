#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(hex, hex_to_bytes)
{
	int ret = 0;
	uint8_t bytes[1];
	char hex[3];
	for (int i = 0; i < (int)sizeof(uint8_t); i++)
	{
		snprintf(hex, sizeof(hex), "%02x", i);

		ret = muggle_hex_to_bytes(hex, bytes, 2);
		ASSERT_EQ(ret, 0);
		ASSERT_EQ(bytes[0], (uint8_t)i);
	}
}

TEST(hex, hex_bytes)
{
	srand((unsigned int)time(NULL));

	uint8_t bytes[8];
	char hex[8*2 + 1];
	for (int i = 0; i < 8; i++)
	{
		bytes[i] = rand() % 256;
	}

	// bytes to hex
	muggle_hex_from_bytes(bytes, hex, 8);

	// hex to bytes
	uint8_t buf[8];
	int ret = muggle_hex_to_bytes(hex, buf, 16);
	ASSERT_EQ(ret, 0);
	for (int i = 0; i < 8; i++)
	{
		ASSERT_EQ(bytes[i], buf[i]);
	}
}
