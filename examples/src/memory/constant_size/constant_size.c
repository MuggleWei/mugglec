#include "muggle/c/muggle_c.h"

typedef struct foo
{
	char buf[32];
	int32_t i32;
} foo_t;

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	muggle_memory_pool_t pool;
	muggle_memory_pool_init(&pool, 8, (unsigned int)sizeof(foo_t));

	muggle_memory_pool_set_flag(&pool, MUGGLE_MEMORY_POOL_CONSTANT_SIZE);

	foo_t *arr[32];
	memset(arr, 0, sizeof(arr));

	for (int i = 0; i < (int)(sizeof(arr)/sizeof(arr[0])); i++)
	{
		arr[i] = (foo_t*)muggle_memory_pool_alloc(&pool);
		if (arr[i] == NULL)
		{
			LOG_INFO("failed allocate #%d block", i);
			break;
		}
		arr[i]->i32 = i;
		LOG_INFO("allocate %d", arr[i]->i32);
	}

	for (int i = 0; i < (int)(sizeof(arr)/sizeof(arr[0])); i++)
	{
		if (arr[i] == NULL)
		{
			LOG_INFO("%d is NULL", i);
			break;
		}
		else
		{
			LOG_INFO("free %d", arr[i]->i32);
			muggle_memory_pool_free(&pool, arr[i]);
		}
	}

	muggle_memory_pool_destroy(&pool);

	return 0;
}