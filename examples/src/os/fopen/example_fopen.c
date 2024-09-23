#include "muggle/c/muggle_c.h"

#define TEST_REL_DIR1 "./tmp/test_mugglec_fopen/rel_filepath1/"
#define TEST_REL_DIR2 "tmp/test_mugglec_fopen/rel_filepath2/"
#if MUGGLE_PLATFORM_WINDOWS
	#define TEST_ABS_DIR "c:/tmp/test_mugglec_fopen/abs_filepath/"
#else
	#define TEST_ABS_DIR "/tmp/test_mugglec_fopen/abs_filepath/"
#endif

static char buf[128];

bool fopen_rel_filepath()
{
	const char *filepath = TEST_REL_DIR1 "hello.txt";
	FILE *fp = NULL;

	fp = muggle_os_fopen(filepath, "w");
	if (fp == NULL) {
		fprintf(stderr, "failed open rel file path: %s\n", filepath);
		return false;
	}
	fprintf(stdout, "success open rel file path: %s\n", filepath);
	fwrite(buf, 1, strlen(buf), fp);
	fclose(fp);

	return true;
}

bool fopen_rel_filepath2()
{
	const char *filepath = TEST_REL_DIR2 "hello.txt";
	FILE *fp = NULL;

	fp = muggle_os_fopen(filepath, "w");
	if (fp == NULL) {
		fprintf(stderr, "failed open rel file path: %s\n", filepath);
		return false;
	}
	fprintf(stdout, "success open rel file path: %s\n", filepath);
	fwrite(buf, 1, strlen(buf), fp);
	fclose(fp);

	return true;
}

bool fopen_abs_filepath()
{
	const char *filepath = TEST_ABS_DIR "hello.txt";
	FILE *fp = NULL;

	fp = muggle_os_fopen(filepath, "w");
	if (fp == NULL) {
		fprintf(stderr, "failed open abs file path: %s\n", filepath);
		return false;
	}
	fprintf(stdout, "success open abs file path: %s\n", filepath);
	fwrite(buf, 1, strlen(buf), fp);
	fclose(fp);

	return true;
}

int main()
{
	struct timespec ts;
	muggle_realtime_get(ts);
	snprintf(buf, sizeof(buf), "%lld.%09lld", (long long)ts.tv_sec,
			(long long)ts.tv_nsec);
	fprintf(stdout, "current ts: %s\n", buf);

	fopen_rel_filepath();
	fopen_rel_filepath2();
	fopen_abs_filepath();

	return 0;
}
