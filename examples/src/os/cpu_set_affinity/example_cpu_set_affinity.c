#include "muggle/c/muggle_c.h"

int main(int argc, char *argv[])
{
	muggle_log_complicated_init(LOG_LEVEL_DEBUG, -1, NULL);

	int cpu = 0;
	if (argc > 1) {
		if (!muggle_str_toi(argv[1], &cpu, 10)) {
			LOG_ERROR("failed convert %s to integer", argv[1]);
			exit(EXIT_FAILURE);
		}
	} else {
		LOG_WARNING("without input cpu number, use '0' by default");
		LOG_WARNING("Usage: %s <CPU number>", argv[0]);
	}

	muggle_pid_handle_t pid = 0;
	int ret = 0;

	// set cpu affinity
	muggle_cpu_mask_t mask;
	muggle_cpu_mask_zero(&mask);
	muggle_cpu_mask_set(&mask, cpu);
	ret = muggle_cpu_set_thread_affinity(pid, &mask);
	if (ret != 0) {
		char errmsg[256];
		muggle_sys_strerror(ret, errmsg, sizeof(errmsg));
		LOG_ERROR("failed set thread's CPU affinity, err=%s", errmsg);
		exit(EXIT_FAILURE);
	}

	// get cpu affinity
	muggle_cpu_mask_zero(&mask);
	ret = muggle_cpu_get_thread_affinity(pid, &mask);
	if (ret != 0) {
		char errmsg[256];
		muggle_sys_strerror(ret, errmsg, sizeof(errmsg));
		LOG_ERROR("failed get thread's CPU affinity, err=%s", errmsg);
		exit(EXIT_FAILURE);
	}

	int n = muggle_thread_hardware_concurrency();
	for (int i = 0; i < n; i++) {
		if (muggle_cpu_mask_isset(&mask, i)) {
			LOG_INFO("CPU #%d | set", i);
		} else {
			LOG_INFO("CPU #%d | not set", i);
		}
	}
	LOG_INFO("run infinite loop; use top/htop to view CPU affinity");

	// infinite loop
	uint32_t cnt = 0;
	while (true) {
		if (cnt++ > 10000000) {
			cnt = 0;
		}
	}

	return 0;
}
