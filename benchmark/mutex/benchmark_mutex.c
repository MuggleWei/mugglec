#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

void func_mutex(void *args, uint64_t idx)
{
	MUGGLE_UNUSED(idx);
	muggle_mutex_t *mutex = (muggle_mutex_t*)args;
	muggle_mutex_lock(mutex);
	muggle_mutex_unlock(mutex);
}

void benchmark_mutex(
	muggle_benchmark_config_t *config,
	fn_muggle_benchmark_func func,
	const char *name)
{
	muggle_mutex_t mutex;
	muggle_mutex_init(&mutex);

	// initialize benchmark memory pool handle
	muggle_benchmark_func_t benchmark;
	muggle_benchmark_func_init(
		&benchmark,
		config,
		&mutex,
		func);

	// run
	muggle_benchmark_func_run(&benchmark);

	// generate report
	muggle_benchmark_func_gen_report(&benchmark, name);

	// destroy benchmark function handle
	muggle_benchmark_func_destroy(&benchmark);

	muggle_mutex_destroy(&mutex);
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
	MUGGLE_LOG_INFO("run mutex-1");
	config.producer = 1;
	benchmark_mutex(&config, func_mutex, "mutex-1");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run mutex-2");
	config.producer = 2;
	benchmark_mutex(&config, func_mutex, "mutex-2");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run mutex-4");
	config.producer = 4;
	benchmark_mutex(&config, func_mutex, "mutex-4");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	int hc = (int)muggle_thread_hardware_concurrency();
	hc /= 2;
	if (hc < 1)
	{
		hc = 1;
	}
	MUGGLE_LOG_INFO("run mutex-half-hc(%d)", hc);
	config.producer = hc;
	benchmark_mutex(&config, func_mutex, "mutex-half-hc");
}
