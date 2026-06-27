#include "muggle/c/muggle_c.h"

int main()
{
	uint32_t cap = 8 * 1024 * 1024;
	uint32_t block_size = 1024;

	{
		muggle_memory_pool_t pool;
		if (!muggle_memory_pool_init(&pool, cap, block_size)) {
			fprintf(stderr, "failed init memory pool\n");
			exit(EXIT_FAILURE);
		}

		// check first and 4G address
		void *p1 = muggle_memory_pool_alloc(&pool);
		for (size_t i = 0; i < cap / 2 - 1; ++i) {
			muggle_memory_pool_alloc(&pool);
		}
		void *p2 = muggle_memory_pool_alloc(&pool);
		if (p1 == p2) {
			fprintf(stderr, "4GB overflow!!!\n");
		} else {
			fprintf(stdout, "it's ok\n");
		}

		muggle_memory_pool_destroy(&pool);
	}

	{
		muggle_memory_pool_t pool;
		if (!muggle_memory_pool_init_thp(&pool, cap, block_size)) {
			fprintf(stderr, "failed init memory pool\n");
			exit(EXIT_FAILURE);
		}

		// check first and 4G address
		void *p1 = muggle_memory_pool_alloc(&pool);
		for (size_t i = 0; i < cap / 2 - 1; ++i) {
			muggle_memory_pool_alloc(&pool);
		}
		void *p2 = muggle_memory_pool_alloc(&pool);
		if (p1 == p2) {
			fprintf(stderr, "4GB overflow!!!\n");
		} else {
			fprintf(stdout, "it's ok\n");
		}

		muggle_memory_pool_destroy(&pool);
	}

	return 0;
}
