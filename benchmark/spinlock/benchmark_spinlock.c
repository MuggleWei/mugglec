#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

void func_spinlock(void *args, uint64_t idx)
{
	MUGGLE_UNUSED(idx);
	muggle_spinlock_t *spinlock = (muggle_spinlock_t*)args;
	muggle_spinlock_lock(spinlock);
	muggle_spinlock_unlock(spinlock);
}

void benchmark_spinlock(
	muggle_benchmark_config_t *config,
	fn_muggle_benchmark_func func,
	const char *name)
{
	muggle_spinlock_t spinlock;
	muggle_spinlock_init(&spinlock);

	// initialize benchmark memory pool handle
	muggle_benchmark_func_t benchmark;
	muggle_benchmark_func_init(
		&benchmark,
		config,
		&spinlock,
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
	MUGGLE_LOG_INFO("run spinlock-1");
	config.producer = 1;
	benchmark_spinlock(&config, func_spinlock, "spinlock-1");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run spinlock-2");
	config.producer = 2;
	benchmark_spinlock(&config, func_spinlock, "spinlock-2");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run spinlock-4");
	config.producer = 4;
	benchmark_spinlock(&config, func_spinlock, "spinlock-4");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	int hc = (int)muggle_thread_hardware_concurrency();
	hc /= 2;
	if (hc < 1)
	{
		hc = 1;
	}
	MUGGLE_LOG_INFO("run spinlock-half-hc(%d)", hc);
	config.producer = hc;
	benchmark_spinlock(&config, func_spinlock, "spinlock-half-hc");
}
