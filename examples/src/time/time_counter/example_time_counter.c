#include "muggle/c/muggle_c.h"

int main()
{
	muggle_log_complicated_init(MUGGLE_LOG_LEVEL_DEBUG, -1, NULL);

	muggle_time_counter_t tc;
	muggle_time_counter_init(&tc);
	for (int i = 0; i < 8; ++i) {
		muggle_time_counter_start(&tc);
		muggle_msleep(1000);
		muggle_time_counter_end(&tc);

		int64_t elapsed_ns = muggle_time_counter_interval_ns(&tc);
		int64_t elapsed_ms = muggle_time_counter_interval_ms(&tc);
		LOG_INFO("elapsed time: ns=%lld, ms=%lld", (long long)elapsed_ns,
				 (long long)elapsed_ms);
	}

	LOG_INFO("bye");

	return 0;
}
