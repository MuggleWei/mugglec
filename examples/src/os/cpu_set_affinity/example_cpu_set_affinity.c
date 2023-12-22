#include "muggle/c/muggle_c.h"

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_DEBUG, -1, NULL);

	muggle_cpu_mask_t mask;
	muggle_cpu_mask_zero(&mask);
	muggle_cpu_mask_set(&mask, 0);
	if (muggle_cpu_set_thread_affinity(0, &mask) != 0) {
		char errmsg[256];
		muggle_sys_strerror(muggle_sys_lasterror(), errmsg, sizeof(errmsg));
		LOG_FATAL("failed set thread's CPU affinity, err=%s", errmsg);
		exit(EXIT_FAILURE);
	}

	uint32_t cnt = 0;
	while (true) {
		if (cnt++ > 10000000) {
			cnt = 0;
			LOG_INFO("run infinite loop, use top/htop to view CPU affinity");
		}
	}

	return 0;
}
