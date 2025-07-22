#include "muggle/c/muggle_c.h"

void case1()
{
	uint32_t cap = 8 * 1024 * 1024;
	uint32_t block_size = 1024;

	muggle_memory_pool_t pool;
	if (!muggle_memory_pool_init(&pool, cap, block_size)) {
		LOG_ERROR("failed init memory pool");
		exit(EXIT_FAILURE);
	}

	LOG_INFO("----------------");
	LOG_INFO("case1 init status");
	LOG_INFO("pool.capacity=%u", pool.capacity);
	LOG_INFO("pool.block_size=%u", pool.block_size);
	LOG_INFO("pool.max_delta_cap=%u", pool.max_delta_cap);

	for (uint32_t i = 0; i < 4 * cap; ++i) {
		void *ptr = muggle_memory_pool_alloc(&pool);
		if (ptr == NULL) {
			LOG_ERROR("failed alloc in index %u", i);
			exit(EXIT_FAILURE);
		}

		uint32_t *p_u32 = (uint32_t *)ptr;
		*p_u32 = i;
	}

	LOG_INFO("----------------");
	LOG_INFO("case1 end status");
	LOG_INFO("pool.capacity=%u", pool.capacity);
	LOG_INFO("pool.block_size=%u", pool.block_size);
	LOG_INFO("pool.max_delta_cap=%u", pool.max_delta_cap);
	LOG_INFO("pool.used=%u", pool.used);

	muggle_memory_pool_destroy(&pool);
}

void case2()
{
	uint32_t cap = 8 * 1024 * 1024;
	uint32_t block_size = 1024;
	uint32_t max_delta_cap = cap;

	muggle_memory_pool_t pool;
	if (!muggle_memory_pool_init(&pool, cap, block_size)) {
		LOG_ERROR("failed init memory pool");
		exit(EXIT_FAILURE);
	}
	muggle_memory_pool_set_max_delta_cap(&pool, max_delta_cap);

	LOG_INFO("----------------");
	LOG_INFO("case2 init status");
	LOG_INFO("pool.capacity=%u", pool.capacity);
	LOG_INFO("pool.block_size=%u", pool.block_size);
	LOG_INFO("pool.max_delta_cap=%u", pool.max_delta_cap);

	for (uint32_t i = 0; i < 4 * cap; ++i) {
		void *ptr = muggle_memory_pool_alloc(&pool);
		if (ptr == NULL) {
			LOG_ERROR("failed alloc in index %u", i);
			exit(EXIT_FAILURE);
		}

		uint32_t *p_u32 = (uint32_t *)ptr;
		*p_u32 = i;
	}

	LOG_INFO("----------------");
	LOG_INFO("case2 end status");
	LOG_INFO("pool.capacity=%u", pool.capacity);
	LOG_INFO("pool.block_size=%u", pool.block_size);
	LOG_INFO("pool.max_delta_cap=%u", pool.max_delta_cap);
	LOG_INFO("pool.used=%u", pool.used);

	muggle_memory_pool_destroy(&pool);
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	case1();
	case2();

	return 0;
}
