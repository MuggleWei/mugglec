#include "muggle/c/muggle_c.h"

typedef struct user_data
{
	int value;
} user_data_t;

void do_free(void *pool, void *data)
{
	free(data);
}

int cmp_int(const void *d1, const void *d2)
{
	return *(int*)d1 - *(int*)d2;
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	muggle_array_list_t array_list;
	muggle_array_list_init(&array_list, 8);

	for (int i = 0; i < 16; i++)
	{
		user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
		data->value = i % 2;
		if (!muggle_array_list_append(&array_list, -1, data))
		{
			LOG_ERROR("failed append data: %d", data->value);
		}
	}

	int v = 1;
	int idx = -1;
	while((idx = muggle_array_list_find(&array_list, idx + 1, &v, cmp_int)) != -1)
	{
		LOG_INFO("find value=%d at index %d", v, idx);
	};

	muggle_array_list_destroy(&array_list, do_free, NULL);

	return 0;
}