#include "muggle/c/muggle_c.h"

void output_usage(FILE *fp, const char *argv0)
{
	fprintf(fp, "Usage: %s <w|r|c>\n", argv0);
	fprintf(fp, "  w: writer\n");
	fprintf(fp, "  r: reader\n");
	fprintf(fp, "  c: cleaner\n");
}

void run_writer(muggle_shm_ringbuf_t *shm_rbuf)
{
	char buf[MUGGLE_CACHE_LINE_SIZE];

	do {
		struct timespec ts;
		struct tm t;
		muggle_realtime_get(ts);

		time_t sec = ts.tv_sec;
		localtime_r(&sec, &t);
		snprintf(buf, sizeof(buf), "%d-%02d-%2dT%02d:%02d:%02d.%09ld",
				 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
				 t.tm_sec, (long)ts.tv_nsec);

		uint32_t nbytes = strlen(buf) + 1;
		void *ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, nbytes);
		if (ptr) {
			memcpy(ptr, buf, nbytes);
			LOG_INFO("write timestamp: %s", buf);

			muggle_shm_ringbuf_w_move(shm_rbuf);
		} else {
			LOG_ERROR("failed alloc bytes for write");
		}

		muggle_nsleep(500 * 1000 * 1000);
	} while (1);
}

void run_reader(muggle_shm_ringbuf_t *shm_rbuf)
{
	do {
		uint32_t n_bytes = 0;
		char *s = (char *)muggle_shm_ringbuf_r_fetch(shm_rbuf, &n_bytes);
		if (s) {
			LOG_INFO("read timestamp: [%u bytes] %s", n_bytes, s);

			muggle_shm_ringbuf_r_move(shm_rbuf);
		}

		muggle_nsleep(1000 * 1000);
	} while (1);
}

void run_cleaner(muggle_shm_t *shm)
{
	if (muggle_shm_detach(shm) != 0) {
		LOG_ERROR("failed shm detach");
		return;
	}
	LOG_INFO("success shm detach");

	if (muggle_shm_rm(shm) != 0) {
		LOG_ERROR("failed shm remove");
		return;
	}
	LOG_INFO("success shm remove");
}

int main(int argc, char *argv[])
{
	// check args
	if (argc != 2) {
		output_usage(stderr, argv[0]);
		exit(EXIT_FAILURE);
	}

	// init log
	if (muggle_log_complicated_init(LOG_LEVEL_DEBUG, -1, NULL) != 0) {
		fprintf(stderr, "failed init log\n");
		exit(EXIT_FAILURE);
	}

	// ensure path exists in *nix
	const char *k_name = "/dev/shm/mugglec_example_shm_ringbuf";
	const int k_num = 5;
#if MUGGLE_PLATFORM_WINDOWS
#else
	FILE *fp = muggle_os_fopen(k_name, "w");
	if (fp == NULL) {
		LOG_ERROR("failed open k_name: %s", k_name);
		exit(EXIT_FAILURE);
	}
	fclose(fp);
#endif

	// open shm ringbuffer
	int flag = MUGGLE_SHM_FLAG_OPEN;
	uint32_t n_bytes = 4096 - sizeof(muggle_shm_ringbuf_t);
	muggle_shm_t shm;
	muggle_shm_ringbuf_t *shm_rbuf =
		muggle_shm_ringbuf_open(&shm, k_name, k_num, flag, n_bytes);
	if (shm_rbuf == NULL) {
		LOG_WARNING("failed open shm_ringbuf, try to create one");

		flag = MUGGLE_SHM_FLAG_CREAT;
		shm_rbuf = muggle_shm_ringbuf_open(&shm, k_name, k_num, flag, n_bytes);
		if (shm_rbuf == NULL) {
			LOG_ERROR("failed create shm_ringbuf");
			exit(EXIT_FAILURE);
		}

		LOG_INFO("success create shm_ringbuf: %s, %d", k_name, k_num);
	} else {
		LOG_INFO("success open shm: %s, %d", k_name, k_num);

		while (!muggle_shm_ringbuf_is_ready(shm_rbuf)) {
			muggle_msleep(100);
		}
	}

	switch (argv[1][0]) {
	case 'w': {
		run_writer(shm_rbuf);
	} break;
	case 'r': {
		run_reader(shm_rbuf);
	} break;
	case 'c': {
		run_cleaner(&shm);
	} break;
	default: {
		LOG_ERROR("invalid role\n");
		output_usage(stderr, argv[0]);
		exit(EXIT_FAILURE);
	} break;
	}

	return 0;
}
