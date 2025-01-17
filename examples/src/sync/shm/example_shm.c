#include "muggle/c/muggle_c.h"
#include <assert.h>

typedef struct {
	union {
		struct {
			uint32_t magic;
			uint32_t nbytes;
			muggle_sync_t ready;
			muggle_spinlock_t lock;
		};
		MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	};
	MUGGLE_STRUCT_CACHE_LINE_X2_PADDING(0);
	char buf[MUGGLE_CACHE_LINE_SIZE];
} data_t;

static_assert(sizeof(data_t) == 2 * MUGGLE_CACHE_LINE_X2_SIZE, "");

void output_usage(FILE *fp, const char *argv0)
{
	fprintf(fp, "Usage: %s <w|r|c>\n", argv0);
	fprintf(fp, "  w: writer\n");
	fprintf(fp, "  r: reader\n");
	fprintf(fp, "  c: cleaner\n");
}

void init_shm_data(data_t *ptr)
{
	memset(ptr, 0, sizeof(*ptr));
	ptr->magic = 0x4D53484D;
	ptr->nbytes = sizeof(data_t);
	muggle_spinlock_init(&ptr->lock);
	muggle_atomic_store(&ptr->ready, 1, muggle_memory_order_release);
}

void run_writer(data_t *ptr)
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

		muggle_spinlock_lock(&ptr->lock);
		static_assert(sizeof(buf) == sizeof(ptr->buf), "");
		memcpy(ptr->buf, buf, sizeof(buf));
		muggle_spinlock_unlock(&ptr->lock);

		LOG_INFO("write timestamp: %s", buf);

		muggle_nsleep(500 * 1000 * 1000);
	} while (1);
}

void run_reader(data_t *ptr)
{
	char buf[MUGGLE_CACHE_LINE_SIZE];
	do {
		muggle_spinlock_lock(&ptr->lock);
		static_assert(sizeof(buf) == sizeof(ptr->buf), "");
		memcpy(buf, ptr->buf, sizeof(buf));
		muggle_spinlock_unlock(&ptr->lock);

		LOG_INFO("read timestamp: %s", buf);

		muggle_nsleep(500 * 1000 * 1000);
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
	const char *k_name = "/dev/shm/mugglec_example_shm";
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

	// open shm
	int flag = MUGGLE_SHM_FLAG_OPEN;
	muggle_shm_t shm;
	data_t *ptr = muggle_shm_open(&shm, k_name, k_num, flag, sizeof(data_t));
	if (ptr == NULL) {
		LOG_WARNING("failed open shm, try to create one");

		flag = MUGGLE_SHM_FLAG_CREAT;
		ptr = muggle_shm_open(&shm, k_name, k_num, flag, sizeof(data_t));
		if (ptr == NULL) {
			LOG_ERROR("failed create shm");
			exit(EXIT_FAILURE);
		}

		LOG_INFO("success create shm: %s, %d", k_name, k_num);

		init_shm_data(ptr);
	} else {
		LOG_INFO("success open shm: %s, %d", k_name, k_num);
	}

	switch (argv[1][0]) {
	case 'w': {
		run_writer(ptr);
	} break;
	case 'r': {
		run_reader(ptr);
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
