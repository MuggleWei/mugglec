#include "endian.h"

int muggle_endianness()
{
	static int16_t i = 1;
	return *(char*)&i == 1 ? MUGGLE_LITTLE_ENDIAN : MUGGLE_BIG_ENDIAN;
}
