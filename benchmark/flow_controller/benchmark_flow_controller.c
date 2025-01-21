#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

void func_flow_ctl(void *args, uint64_t idx)
{
	MUGGLE_UNUSED(idx);
	muggle_flow_controller_t *flow_ctl = (muggle_flow_controller_t *)args;
	muggle_flow_ctl_check_and_force_update(flow_ctl);
}
void benchmark_flow_ctl(muggle_benchmark_config_t *config, const char *name)
{
	LOG_INFO("run %s", name);

	muggle_flow_controller_t flow_ctl;
	muggle_flow_ctl_init(&flow_ctl, 60, 100, 3600);

	// initialize benchmark memory pool handle
	muggle_benchmark_func_t benchmark;
	muggle_benchmark_func_init(&benchmark, config, &flow_ctl, func_flow_ctl);

	// run
	muggle_benchmark_func_run(&benchmark);

	// generate report
	muggle_benchmark_func_gen_report(&benchmark, name);

	// destroy benchmark function handle
	muggle_benchmark_func_destroy(&benchmark);

	// cleanup
	muggle_flow_ctl_destroy(&flow_ctl);
}

void func_fast_flow_ctl(void *args, uint64_t idx)
{
	MUGGLE_UNUSED(idx);
	muggle_fast_flow_controller_t *fast_flow_ctl =
		(muggle_fast_flow_controller_t *)args;
	muggle_fast_flow_ctl_check_and_force_update(fast_flow_ctl);
}
void benchmark_fast_flow_ctl(muggle_benchmark_config_t *config,
							 const char *name)
{
	LOG_INFO("run %s", name);

	muggle_fast_flow_controller_t fast_flow_ctl;
	muggle_fast_flow_ctl_init(&fast_flow_ctl, 60, 100, 3600,
							  muggle_rdtsc_freq_calibrate());

	// initialize benchmark memory pool handle
	muggle_benchmark_func_t benchmark;
	muggle_benchmark_func_init(&benchmark, config, &fast_flow_ctl,
							   func_fast_flow_ctl);

	// run
	muggle_benchmark_func_run(&benchmark);

	// generate report
	muggle_benchmark_func_gen_report(&benchmark, name);

	// destroy benchmark function handle
	muggle_benchmark_func_destroy(&benchmark);

	// cleanup
	muggle_fast_flow_ctl_destroy(&fast_flow_ctl);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);
	config.producer = 1;
	muggle_benchmark_config_output(&config);

	benchmark_flow_ctl(&config, "flow_controller");

#if MUGGLE_SUPPORT_FAST_FLOW_CONTROLLER
	benchmark_fast_flow_ctl(&config, "fast_flow_controller");
#endif

	return 0;
}
