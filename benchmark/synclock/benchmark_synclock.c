#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

#if MUGGLE_C_HAVE_SYNC_OBJ

void func_synclock(void *args, uint64_t idx)
{
	MUGGLE_UNUSED(idx);
	muggle_sync_t *synclock = (muggle_sync_t*)args;
	muggle_synclock_lock(synclock);
	muggle_synclock_unlock(synclock);
}

void benchmark_synclock(
	muggle_benchmark_config_t *config,
	fn_muggle_benchmark_func func,
	const char *name)
{
	muggle_sync_t synclock;
	muggle_synclock_init(&synclock);

	// initialize benchmark memory pool handle
	muggle_benchmark_func_t benchmark;
	muggle_benchmark_func_init(
		&benchmark,
		config,
		&synclock,
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
	MUGGLE_LOG_INFO("run synclock-1");
	config.producer = 1;
	benchmark_synclock(&config, func_synclock, "synclock-1");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run synclock-2");
	config.producer = 2;
	benchmark_synclock(&config, func_synclock, "synclock-2");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run synclock-4");
	config.producer = 4;
	benchmark_synclock(&config, func_synclock, "synclock-4");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	int hc = (int)muggle_thread_hardware_concurrency();
	hc /= 2;
	if (hc < 1)
	{
		hc = 1;
	}
	MUGGLE_LOG_INFO("run synclock-half-hc(%d)", hc);
	config.producer = hc;
	benchmark_synclock(&config, func_synclock, "synclock-half-hc");
}

#else

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	MUGGLE_LOG_INFO("MUGGLE_C_HAVE_SYNC_OBJ is 0");

	return 0;
}

#endif
