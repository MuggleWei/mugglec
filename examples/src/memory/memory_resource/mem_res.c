#include "muggle/c/muggle_c.h"
#include <inttypes.h>

// #define N_BYTES (1 * MUGGLE_MEMORY_RES_PAGE_SIZE_1GB)
#define N_BYTES (32 * MUGGLE_MEMORY_RES_PAGE_SIZE_2MB)
#define BLOCKSIZE 512

void run_alloc(muggle_memory_resource_t *res)
{
	size_t cnt = 0;
	void *p = NULL;
	while (true) {
		p = muggle_memory_res_alloc(res, BLOCKSIZE);
		if (p == NULL) {
			break;
		}

		++cnt;
	}

	LOG_INFO("total allocate %lu blocksize", (unsigned long)cnt);
}

void run_default()
{
	// init
	muggle_memory_resource_t res;
	muggle_memory_res_flags_t flags;
	memset(&flags, 0, sizeof(flags));
	if (!muggle_memory_res_init(&res, N_BYTES, flags.val, NULL, 0)) {
		LOG_ERROR("failed init memory resource with default flags");
		return;
	}

	LOG_INFO("success init memory resource with default flags");

	// run alloc
	run_alloc(&res);

	// cleanup
	muggle_memory_res_destroy(&res);

	LOG_INFO("cleanup memory resource");
}

void run_huge_private()
{
	// init
	muggle_memory_resource_t res;
	muggle_memory_res_flags_t flags;
	memset(&flags, 0, sizeof(flags));
	flags.mem_type = MUGGLE_MEMORY_RES_TYPE_HUGE_PRIVATE;
	flags.populate_type = MUGGLE_MEMORY_RES_POPULATE_WRITE;
	if (!muggle_memory_res_init(&res, N_BYTES, flags.val, NULL, 0)) {
		LOG_ERROR("failed init memory resource with huge private flags");
		return;
	}

	LOG_INFO("success init memory resource with huge private flags");

	// run alloc
	run_alloc(&res);

	// cleanup
	muggle_memory_res_destroy(&res);

	LOG_INFO("cleanup memory resource");
}

void run_huge_share()
{
	const char *k_name = "/dev/shm/mugglec_example_memres";
#if MUGGLE_PLATFORM_LINUX
	FILE *fp = muggle_os_fopen(k_name, "w");
	if (fp == NULL) {
		LOG_ERROR("failed open k_name: %s", k_name);
		return;
	}
	fclose(fp);
#endif
	int k_num = 5;

	// init
	muggle_memory_resource_t res;
	muggle_memory_res_flags_t flags;
	memset(&flags, 0, sizeof(flags));
	flags.mem_type = MUGGLE_MEMORY_RES_TYPE_HUGE_SHARE;
	flags.populate_type = MUGGLE_MEMORY_RES_POPULATE_WRITE;
	flags.share_flag = MUGGLE_MEMORY_RES_SHM_CREATE;
	if (!muggle_memory_res_init(&res, N_BYTES, flags.val, k_name, k_num)) {
		LOG_ERROR(
			"failed init memory resource with huge share flags and create, "
			"errno=%d",
			MUGGLE_EVENT_LAST_ERRNO);

		flags.share_flag = 0;
		if (!muggle_memory_res_init(&res, N_BYTES, flags.val, k_name, k_num)) {
			LOG_ERROR(
				"failed init memory resource with huge share flags and open, "
				"errno=%d",
				MUGGLE_EVENT_LAST_ERRNO);
			return;
		}
	}

	LOG_INFO("success init memory resource with huge share flags");

	// run alloc
	run_alloc(&res);

	// cleanup
	muggle_memory_res_destroy(&res);
	LOG_INFO("cleanup memory resource");

	// rm shm
	if (!muggle_memory_res_rm_shm(k_name, k_num)) {
		LOG_ERROR("failed rm shm");
	} else {
		LOG_INFO("cleanup share memory");
	}
}

void run_huge_thp()
{
	// init
	muggle_memory_resource_t res;
	muggle_memory_res_flags_t flags;
	memset(&flags, 0, sizeof(flags));
	flags.mem_type = MUGGLE_MEMORY_RES_TYPE_HUGE_THP;
	flags.populate_type = MUGGLE_MEMORY_RES_POPULATE_WRITE;
	if (!muggle_memory_res_init(&res, N_BYTES, flags.val, NULL, 0)) {
		LOG_ERROR("failed init memory resource with huge thp flags");
		return;
	}

	LOG_INFO("success init memory resource with huge thp flags");

	// run alloc
	run_alloc(&res);

	// cleanup
	muggle_memory_res_destroy(&res);

	LOG_INFO("cleanup memory resource");
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	run_default();

	run_huge_private();

	run_huge_share();

	run_huge_thp();

	return 0;
}
