#include "muggle/c/muggle_c.h"

#define DATA_SIZE \
	(MUGGLE_CACHE_LINE_SIZE - sizeof(muggle_shm_ringbuf_data_hdr_t))

#define N 100000

typedef union {
	struct {
		struct timespec ts_before_alloc;
		struct timespec ts_before_w;
		struct timespec ts_after_r;
	};
	char placeholder[DATA_SIZE];
} data_t;

void output_usage(FILE *fp, const char *argv0)
{
	fprintf(fp, "Usage: %s <w|r|c>\n", argv0);
	fprintf(fp, "  w: writer\n");
	fprintf(fp, "  r: reader\n");
	fprintf(fp, "  c: cleaner\n");
}

void run_writer(muggle_shm_ringbuf_t *shm_rbuf)
{
	uint32_t n = 0;
	struct timespec ts_before_alloc;
	do {
		muggle_realtime_get(ts_before_alloc);

		data_t *ptr =
			muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, sizeof(data_t));
		if (ptr) {
			memcpy(&ptr->ts_before_alloc, &ts_before_alloc,
				   sizeof(struct timespec));
			muggle_realtime_get(ptr->ts_before_w);
			muggle_shm_ringbuf_w_move(shm_rbuf);

			++n;
			if (n % 10000 == 0) {
				LOG_INFO("already write: %u", n);
			}
			if (n == N) {
				break;
			}
		} else {
			LOG_ERROR("failed alloc bytes for write");
			muggle_msleep(1000);
		}

		muggle_nsleep(1000);
	} while (1);

	LOG_INFO("write mission completed");
}

static data_t s_datas[N];

int64_t cal_elapsed_ns(struct timespec *ts1, struct timespec *ts2)
{
	return (ts2->tv_sec - ts1->tv_sec) * 1000000000LL + ts2->tv_nsec -
		   ts1->tv_nsec;
}

void write_statistics_head(FILE *fp)
{
	for (int i = 0; i < 100; i += 10) {
		fprintf(fp, "%d,", i);
	}
	fprintf(fp, "100\n");
}
void write_statistics(FILE *fp, int64_t *elapseds, int n)
{
	for (int i = 0; i < 100; i += 10) {
		uint64_t idx = (uint64_t)((i / 100.0) * n);
		fprintf(fp, "%llu,", (unsigned long long)elapseds[idx]);
	}
	fprintf(fp, "%llu\n", (unsigned long long)elapseds[n - 1]);
}

static int compare_int64(const void *a, const void *b)
{
	const int64_t arg1 = *(const int64_t *)a;
	const int64_t arg2 = *(const int64_t *)b;

	if (arg1 < arg2)
		return -1;
	if (arg1 > arg2)
		return 1;
	return 0;
}

void gen_report()
{
	FILE *fp = NULL;
	int64_t elapseds[N];

	// write records
	const char *records_filepath = "./report_shm_ringbuf_records.txt";
	LOG_INFO("gen report records: %s", records_filepath);
	fp = muggle_os_fopen(records_filepath, "w");
	for (int i = 0; i < N; ++i) {
		fprintf(fp, "[%d] before alloc: %lld,%09ld\n", i,
				(long long)s_datas[i].ts_before_alloc.tv_sec,
				(long)s_datas[i].ts_before_alloc.tv_nsec);
		fprintf(fp, "[%d] before write: %lld,%09ld\n", i,
				(long long)s_datas[i].ts_before_w.tv_sec,
				(long)s_datas[i].ts_before_w.tv_nsec);
		fprintf(fp, "[%d] after read: %lld,%09ld\n", i,
				(long long)s_datas[i].ts_before_w.tv_sec,
				(long)s_datas[i].ts_before_w.tv_nsec);
	}
	fclose(fp);

	// write statistics: alloc -> write
	LOG_INFO("gen report alloc -> write: %s", records_filepath);
	const char *alloc_write_filepath = "./report_shm_ringbuf_alloc_write.csv";
	for (int i = 0; i < N; ++i) {
		elapseds[i] = cal_elapsed_ns(&s_datas[i].ts_before_alloc,
									 &s_datas[i].ts_before_w);
	}

	fp = muggle_os_fopen(alloc_write_filepath, "w");
	write_statistics_head(fp);
	write_statistics(fp, elapseds, N);
	qsort(elapseds, N, sizeof(int64_t), compare_int64);
	write_statistics(fp, elapseds, N);
	fclose(fp);

	// write statistics: write -> read
	LOG_INFO("gen report write -> read : %s", records_filepath);
	const char *write_read_filepath = "./report_shm_ringbuf_write_read.csv";
	for (int i = 0; i < N; ++i) {
		elapseds[i] =
			cal_elapsed_ns(&s_datas[i].ts_before_w, &s_datas[i].ts_after_r);
	}

	fp = muggle_os_fopen(write_read_filepath, "w");
	write_statistics_head(fp);
	write_statistics(fp, elapseds, N);
	qsort(elapseds, N, sizeof(int64_t), compare_int64);
	write_statistics(fp, elapseds, N);
	fclose(fp);

	// write statistics: alloc -> read
	LOG_INFO("gen report alloc -> read : %s", records_filepath);
	const char *alloc_read_filepath = "./report_shm_ringbuf_alloc_read.csv";
	for (int i = 0; i < N; ++i) {
		elapseds[i] =
			cal_elapsed_ns(&s_datas[i].ts_before_alloc, &s_datas[i].ts_after_r);
	}

	fp = muggle_os_fopen(alloc_read_filepath, "w");
	write_statistics_head(fp);
	write_statistics(fp, elapseds, N);
	qsort(elapseds, N, sizeof(int64_t), compare_int64);
	write_statistics(fp, elapseds, N);
	fclose(fp);
}

void run_reader(muggle_shm_ringbuf_t *shm_rbuf)
{
	uint32_t n = 0;
	do {
		uint32_t n_bytes = 0;
		data_t *ptr = (data_t *)muggle_shm_ringbuf_r_fetch(shm_rbuf, &n_bytes);
		if (ptr) {
			muggle_realtime_get(ptr->ts_after_r);
			memcpy(&s_datas[n], ptr, sizeof(data_t));
			muggle_shm_ringbuf_r_move(shm_rbuf);

			if (++n == N) {
				break;
			}
		}
	} while (1);

	gen_report();
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
	if (!muggle_path_exists(k_name)) {
		FILE *fp = muggle_os_fopen(k_name, "w");
		if (fp == NULL) {
			LOG_ERROR("failed open k_name: %s", k_name);
			exit(EXIT_FAILURE);
		}
		fclose(fp);
	}
#endif

	uint32_t n_bytes = 4 * 1024 * 1024;

	// open shm ringbuffer
	int flag = MUGGLE_SHM_FLAG_OPEN;
	muggle_shm_t shm;
	muggle_shm_ringbuf_t *shm_rbuf =
		muggle_shm_ringbuf_open(&shm, k_name, k_num, flag, n_bytes);
	if (shm_rbuf == NULL) {
		if (argv[1][0] == 'w') {
			LOG_WARNING("failed open shm_ringbuf, run reader first");
			exit(EXIT_FAILURE);
		}

		LOG_WARNING("failed open shm_ringbuf, try to create one");

		flag = MUGGLE_SHM_FLAG_CREAT;
		shm_rbuf = muggle_shm_ringbuf_open(&shm, k_name, k_num, flag, n_bytes);
		if (shm_rbuf == NULL) {
			LOG_ERROR("failed create shm_ringbuf");
			exit(EXIT_FAILURE);
		}

		LOG_INFO("success create shm_ringbuf: %s, %d", k_name, k_num);
		LOG_INFO("shm_ringbuf.n_cacheline: %u", shm_rbuf->n_cacheline);
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
		run_cleaner(&shm);
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

#if MUGGLE_PLATFORM_WINDOWS
#else
	if (argv[1][0] == 'r') {
		muggle_os_remove(k_name);
	}
#endif

	return 0;
}
