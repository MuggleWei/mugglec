#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

#define TOTAL_CNT 1024

typedef struct {
	uint32_t u32;
} data_t;

void fn_backend_callback(muggle_ma_ring_t *ring, void *data)
{
	MUGGLE_UNUSED(ring);
	MUGGLE_UNUSED(data);
}

void func_ma_ring_write(void *args, uint64_t idx)
{
	muggle_ma_ring_t *ring = (muggle_ma_ring_t*)args;

	data_t *data = (data_t *)muggle_ma_ring_alloc(ring);
	data->u32 = idx;
	muggle_ma_ring_move(ring);
}

void benchmark_ma_ring(muggle_benchmark_config_t *config,
					   fn_muggle_benchmark_func func, const char *name)
{
	muggle_ma_ring_ctx_set_capacity(TOTAL_CNT);
	muggle_ma_ring_ctx_set_data_size(sizeof(data_t));
	muggle_ma_ring_ctx_set_callback(fn_backend_callback);
	muggle_ma_ring_backend_run();

	muggle_ma_ring_thread_ctx_init();

	muggle_ma_ring_t *ring = muggle_ma_ring_thread_ctx_get();

	// initialize benchmark handle
	muggle_benchmark_func_t benchmark;
	muggle_benchmark_func_init(
		&benchmark,
		config,
		ring,
		func);

	// run
	muggle_benchmark_func_run(&benchmark);

	// generate report
	muggle_benchmark_func_gen_report(&benchmark, name);

	// destroy benchmark function handle
	muggle_benchmark_func_destroy(&benchmark);

	muggle_ma_ring_thread_ctx_cleanup();
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

	benchmark_ma_ring(&config, func_ma_ring_write, "ma_ring");

	return 0;
}
