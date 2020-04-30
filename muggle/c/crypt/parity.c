/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "parity.h"
#include <stdio.h>

static const unsigned char s_odd_parity[256] = {
	1, 1, 2, 2, 4, 4, 7, 7, 8, 8, 11, 11, 13, 13, 14, 14,
	16, 16, 19, 19, 21, 21, 22, 22, 25, 25, 26, 26, 28, 28, 31, 31,
	32, 32, 35, 35, 37, 37, 38, 38, 41, 41, 42, 42, 44, 44, 47, 47,
	49, 49, 50, 50, 52, 52, 55, 55, 56, 56, 59, 59, 61, 61, 62, 62,
	64, 64, 67, 67, 69, 69, 70, 70, 73, 73, 74, 74, 76, 76, 79, 79,
	81, 81, 82, 82, 84, 84, 87, 87, 88, 88, 91, 91, 93, 93, 94, 94,
	97, 97, 98, 98, 100, 100, 103, 103, 104, 104, 107, 107, 109, 109, 110, 110,
	112, 112, 115, 115, 117, 117, 118, 118, 121, 121, 122, 122, 124, 124, 127, 127,
	128, 128, 131, 131, 133, 133, 134, 134, 137, 137, 138, 138, 140, 140, 143, 143,
	145, 145, 146, 146, 148, 148, 151, 151, 152, 152, 155, 155, 157, 157, 158, 158,
	161, 161, 162, 162, 164, 164, 167, 167, 168, 168, 171, 171, 173, 173, 174, 174,
	176, 176, 179, 179, 181, 181, 182, 182, 185, 185, 186, 186, 188, 188, 191, 191,
	193, 193, 194, 194, 196, 196, 199, 199, 200, 200, 203, 203, 205, 205, 206, 206,
	208, 208, 211, 211, 213, 213, 214, 214, 217, 217, 218, 218, 220, 220, 223, 223,
	224, 224, 227, 227, 229, 229, 230, 230, 233, 233, 234, 234, 236, 236, 239, 239,
	241, 241, 242, 242, 244, 244, 247, 247, 248, 248, 251, 251, 253, 253, 254, 254
};

static const unsigned char s_even_parity[256] = {
	0, 0, 3, 3, 5, 5, 6, 6, 9, 9, 10, 10, 12, 12, 15, 15,
	17, 17, 18, 18, 20, 20, 23, 23, 24, 24, 27, 27, 29, 29, 30, 30,
	33, 33, 34, 34, 36, 36, 39, 39, 40, 40, 43, 43, 45, 45, 46, 46,
	48, 48, 51, 51, 53, 53, 54, 54, 57, 57, 58, 58, 60, 60, 63, 63,
	65, 65, 66, 66, 68, 68, 71, 71, 72, 72, 75, 75, 77, 77, 78, 78,
	80, 80, 83, 83, 85, 85, 86, 86, 89, 89, 90, 90, 92, 92, 95, 95,
	96, 96, 99, 99, 101, 101, 102, 102, 105, 105, 106, 106, 108, 108, 111, 111,
	113, 113, 114, 114, 116, 116, 119, 119, 120, 120, 123, 123, 125, 125, 126, 126,
	129, 129, 130, 130, 132, 132, 135, 135, 136, 136, 139, 139, 141, 141, 142, 142,
	144, 144, 147, 147, 149, 149, 150, 150, 153, 153, 154, 154, 156, 156, 159, 159,
	160, 160, 163, 163, 165, 165, 166, 166, 169, 169, 170, 170, 172, 172, 175, 175,
	177, 177, 178, 178, 180, 180, 183, 183, 184, 184, 187, 187, 189, 189, 190, 190,
	192, 192, 195, 195, 197, 197, 198, 198, 201, 201, 202, 202, 204, 204, 207, 207,
	209, 209, 210, 210, 212, 212, 215, 215, 216, 216, 219, 219, 221, 221, 222, 222,
	225, 225, 226, 226, 228, 228, 231, 231, 232, 232, 235, 235, 237, 237, 238, 238,
	240, 240, 243, 243, 245, 245, 246, 246, 249, 249, 250, 250, 252, 252, 255, 255
};

/*
 * generate parity check array, and output
 * @odd: 0x01 - odd parity, 0x00 - even parity
 * */
static void muggle_parity_gen(unsigned char odd)
{
	unsigned char c;
	int cnt = 0;
	for (int i = 0; i < 256; ++i)
	{
		c = (unsigned char)i;
		c >>= 1;
		unsigned char mask = 0x01;
		unsigned char parity = odd;
		for (int j = 0; j < 7; ++j)
		{
			if ((mask << j) & c)
			{
				parity ^= 0x01;
			}
		}
		c = (c << 1) | parity;
		printf("%d, ", (unsigned int)c);

		if (++cnt == 16)
		{
			cnt = 0;
			printf("\n");
		}
	}
}

void muggle_output_odd_parity()
{
	printf("odd parity\n");
	muggle_parity_gen(0x01);
}

void output_even_parity()
{
	printf("even parity\n");
	muggle_parity_gen(0x00);
}

int muggle_parity_check_odd(unsigned char b)
{
	return b == s_odd_parity[b] ? 1 : 0;
}
int muggle_parity_check_even(unsigned char b)
{
	return b == s_even_parity[b] ? 1 : 0;
}

unsigned char muggle_parity_set_odd(unsigned char b)
{
	return s_odd_parity[b];
}
unsigned char muggle_parity_set_even(unsigned char b)
{
	return s_even_parity[b];
}