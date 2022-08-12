#include "muggle/c/muggle_c.h"

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, "log/dbg_rel.log");

	DEBUG_LOG_DEBUG("debug debug");
	DEBUG_LOG_INFO("debug info");
	DEBUG_LOG_WARNING("debug warning");

	LOG_DEBUG("debug & release debug");
	LOG_INFO("debug & release info");
	LOG_WARNING("debug & release warning");

	return 0;
}