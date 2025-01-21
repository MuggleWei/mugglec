#include "muggle/c/muggle_c.h"

void OutputResult(bool ret)
{
	struct timespec ts;
	muggle_realtime_get(ts);

	struct tm t;
	time_t sec = ts.tv_sec;
	localtime_r(&sec, &t);
	if (ret) {
		LOG_INFO("timestamp: %d-%02d-%02dT%02d:%02d:%02d.%09lld | Pass",
				 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
				 t.tm_sec, (long long)ts.tv_nsec);
	} else {
		LOG_WARNING("timestamp: %d-%02d-%02dT%02d:%02d:%02d.%09lld | Reject",
					t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour,
					t.tm_min, t.tm_sec, (long long)ts.tv_nsec);
	}
}

void run_flow_ctl_check_and_update_sec_t_cnt_n(int64_t t, uint32_t n)
{
	LOG_INFO("--------------------------------");
	LOG_INFO("flow control: %u requests per %lld seconds", n, (long long)t);

	muggle_flow_controller_t flow_ctl;
	if (!muggle_flow_ctl_init(&flow_ctl, t, n, 3600)) {
		LOG_ERROR("failed init flow controller");
		return;
	}

	for (uint32_t i = 0; i < n; ++i) {
		bool ret = muggle_flow_ctl_check_and_update(&flow_ctl);
		OutputResult(ret);
	}

	for (int64_t sec = 0; sec <= t; ++sec) {
		bool ret = muggle_flow_ctl_check_and_update(&flow_ctl);
		OutputResult(ret);
		muggle_msleep(1000);
	}

	muggle_flow_ctl_destroy(&flow_ctl);
}

void run_flow_ctl_check_and_force_update_sec_t_cnt_n(int64_t t, uint32_t n)
{
	LOG_INFO("--------------------------------");
	LOG_INFO("flow control(force update): %u requests per %lld seconds", n,
			 (long long)t);

	muggle_flow_controller_t flow_ctl;
	if (!muggle_flow_ctl_init(&flow_ctl, t, n, 3600)) {
		LOG_ERROR("failed init flow controller");
		return;
	}

	for (uint32_t i = 0; i < n; ++i) {
		bool ret = muggle_flow_ctl_check_and_force_update(&flow_ctl);
		OutputResult(ret);
	}

	for (int64_t sec = 0; sec <= t; ++sec) {
		bool ret = muggle_flow_ctl_check_and_force_update(&flow_ctl);
		OutputResult(ret);
		muggle_msleep(1000);
	}

	muggle_flow_ctl_destroy(&flow_ctl);
}

int main()
{
	muggle_log_complicated_init(MUGGLE_LOG_LEVEL_DEBUG, -1, NULL);

	run_flow_ctl_check_and_update_sec_t_cnt_n(1, 5);
	run_flow_ctl_check_and_update_sec_t_cnt_n(5, 1);

	run_flow_ctl_check_and_force_update_sec_t_cnt_n(5, 1);

	return 0;
}
