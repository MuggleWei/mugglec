#include "muggle/c/muggle_c.h"

typedef struct foo {
	char buf[32];
	int32_t i32;
} foo_t;

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	muggle_memory_pool_t pool;
	if (!muggle_memory_pool_init_thp(&pool, 2 * 1024 * 1024,
									 (unsigned int)sizeof(foo_t))) {
		LOG_ERROR("failed init memory pool THP");
		exit(EXIT_FAILURE);
	}

	static foo_t *arr[8 * 1024 * 1024];
	for (int i = 0; i < (int)(sizeof(arr) / sizeof(arr[0])); i++) {
		arr[i] = (foo_t *)muggle_memory_pool_alloc(&pool);
		arr[i]->i32 = i;
		if (i % (1024 * 1024) == 0) {
			LOG_INFO("allocate %d", arr[i]->i32);
		}
	}

	for (int i = 0; i < (int)(sizeof(arr) / sizeof(arr[0])); i++) {
		if (i % (1024 * 1024) == 0) {
			LOG_INFO("free %d", arr[i]->i32);
		}
		muggle_memory_pool_free(&pool, arr[i]);
	}

	muggle_memory_pool_destroy(&pool);

	return 0;
}
