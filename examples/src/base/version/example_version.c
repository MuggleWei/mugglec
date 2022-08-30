#include "muggle/c/muggle_c.h"

int main()
{
	muggle_log_complicated_init(MUGGLE_LOG_LEVEL_DEBUG, -1, NULL);

	LOG_INFO("mugglec version: %s", mugglec_version());
	LOG_INFO("mugglec compile time(preprocessor macro): %s", mugglec_compile_time());

	char buf[21];
	const char *compile_dt = mugglec_compile_time_iso8601(buf, sizeof(buf));
	LOG_INFO("mugglec compile datetime(ISO 8601): %s", compile_dt);

	return 0;
}

