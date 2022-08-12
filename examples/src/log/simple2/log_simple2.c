#include "muggle/c/muggle_c.h"

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, "log/log_simple2.log");
	LOG_INFO("hello world");

	return 0;
}