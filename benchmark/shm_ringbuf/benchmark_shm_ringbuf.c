#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

typedef struct {
	void *ptr;
} data_t;

int shm_ringbuf_write(void *user_args, void *data)
{
	muggle_shm_ringbuf_t *shm_rbuf = (muggle_shm_ringbuf_t *)user_args;
	void **ptr = NULL;
	do {
		ptr =
			(void **)muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, sizeof(void *));
	} while (ptr == NULL);
	*ptr = data;
	muggle_shm_ringbuf_w_move(shm_rbuf);

	return 0;
}

void *shm_ringbuf_read(void *user_args, int consumer_id)
{
	MUGGLE_UNUSED(consumer_id);
	muggle_shm_ringbuf_t *shm_rbuf = (muggle_shm_ringbuf_t *)user_args;
	uint32_t n_bytes = 0;
	void **ptr = NULL;
	void *data = NULL;
	do {
		ptr = (void**)muggle_shm_ringbuf_r_fetch(shm_rbuf, &n_bytes);
	} while (ptr == NULL);
	data = *ptr;
	muggle_shm_ringbuf_r_move(shm_rbuf);

	return data;
}

void producer_complete_cb(muggle_benchmark_config_t *config, void *user_args)
{
	MUGGLE_UNUSED(config);

	static muggle_benchmark_thread_message_t end_msg;
	memset(&end_msg, 0, sizeof(end_msg));
	end_msg.id = UINT64_MAX;
	shm_ringbuf_write(user_args, (void *)&end_msg);
}

void benchmark_shm_ringbuf(muggle_benchmark_config_t *config, const char *name)
{
	const char *k_name = "/dev/shm/mugglec_shm_ringbuf_benchmark";
	int k_num = 5;
	const uint32_t n_bytes = 4 * 1024 * 1024;

	// if in *nux, ensure k_name is exists path
#if MUGGLE_PLATFORM_WINDOWS
#else
	if (!muggle_path_exists(k_name)) {
		FILE *fp = muggle_os_fopen(k_name, "w");
		if (fp == NULL) {
			LOG_ERROR("failed create path: %s", k_name);
			exit(EXIT_FAILURE);
		}
		fclose(fp);
	}
#endif

	// prepare shm_ringbuf
	muggle_shm_t shm;
	muggle_shm_ringbuf_t *shm_rbuf = NULL;
	if (muggle_shm_open(&shm, k_name, k_num, MUGGLE_SHM_FLAG_OPEN, 0) != NULL) {
		muggle_shm_detach(&shm);
		muggle_shm_rm(&shm);
	}
	shm_rbuf = muggle_shm_ringbuf_open(&shm, k_name, k_num,
									   MUGGLE_SHM_FLAG_CREAT, n_bytes);

	// initialize thread transfer benchmark
	muggle_benchmark_thread_trans_t benchmark;
	muggle_benchmark_thread_trans_init(&benchmark, config, (void *)shm_rbuf,
									   shm_ringbuf_write, shm_ringbuf_read,
									   producer_complete_cb);

	// run benchmark
	muggle_benchmark_thread_trans_run(&benchmark);

	// generate report
	muggle_benchmark_thread_trans_gen_report(&benchmark, name);

	// destroy benchmark
	muggle_benchmark_thread_trans_destroy(&benchmark);

	// cleanup shm_ringbuf
	muggle_shm_detach(&shm);
	muggle_shm_rm(&shm);

#if MUGGLE_PLATFORM_WINDOWS
#else
	muggle_os_remove(k_name);
#endif
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);

	config.consumer = 1;
	config.producer = 1;

	muggle_benchmark_config_output(&config);

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run shm_ringbuf");
	benchmark_shm_ringbuf(&config, "shm_ringbuf_1_w_1_r");
}
