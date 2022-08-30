#include "muggle/c/muggle_c.h"

int main()
{
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_DEBUG);
	MUGGLE_LOG_INFO("hello world");

	return 0;
}