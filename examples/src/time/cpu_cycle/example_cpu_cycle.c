#include "muggle/c/muggle_c.h"

int main()
{
	muggle_log_complicated_init(MUGGLE_LOG_LEVEL_DEBUG, -1, NULL);

	double ticks_per_sec = muggle_rdtsc_freq_calibrate();

	uint64_t start_ticks = muggle_rdtsc();
	muggle_nsleep(1000);
	uint64_t end_ticks = muggle_rdtsc();
	double elapsed_ns =
		muggle_tsc_elapsed_ns(start_ticks, end_ticks, ticks_per_sec);

	LOG_INFO("tsc ticks per second: %f", ticks_per_sec);
	LOG_INFO("elapsed ns: %f", elapsed_ns);

	return 0;
}
