#include "muggle/c/muggle_c.h"

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, "log/assert.log");

	int v = 0;
	MUGGLE_ASSERT_MSG(v == 1, "ohh!");

	return 0;
}