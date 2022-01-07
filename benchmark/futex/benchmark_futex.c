#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

#if MUGGLE_SUPPORT_FUTEX

void func_futex(void *args, uint64_t idx)
{
	muggle_atomic_int *futex = (muggle_atomic_int*)args;
	muggle_futex_lock(futex);
	muggle_futex_unlock(futex);
	muggle_futex_wake_one(futex);
}

void benchmark_futex(
	muggle_benchmark_config_t *config,
	fn_muggle_benchmark_func func,
	const char *name)
{
	muggle_atomic_int futex;
	muggle_futex_init(&futex);

	// initialize benchmark memory pool handle
	muggle_benchmark_func_t benchmark;
	muggle_benchmark_func_init(
		&benchmark,
		config,
		&futex,
		func);

	// run
	muggle_benchmark_func_run(&benchmark);

	// generate report
	muggle_benchmark_func_gen_report(&benchmark, name);

	// destroy benchmark function handle
	muggle_benchmark_func_destroy(&benchmark);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);
	config.producer = 0;
	muggle_benchmark_config_output(&config);

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run futex-1");
	config.producer = 1;
	benchmark_futex(&config, func_futex, "futex-1");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run futex-2");
	config.producer = 2;
	benchmark_futex(&config, func_futex, "futex-2");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run futex-4");
	config.producer = 4;
	benchmark_futex(&config, func_futex, "futex-4");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	int hc = (int)muggle_thread_hardware_concurrency();
	hc /= 2;
	if (hc < 1)
	{
		hc = 1;
	}
	MUGGLE_LOG_INFO("run futex-half-hc(%d)", hc);
	config.producer = hc;
	benchmark_futex(&config, func_futex, "futex-half-hc");
}

#else

int main()
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	MUGGLE_LOG_ERROR("futex not support in this platform");

	return 0;
}

#endif
