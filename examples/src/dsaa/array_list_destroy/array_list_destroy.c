#include "muggle/c/muggle_c.h"

typedef struct user_data
{
	int value;
} user_data_t;

void do_free(void *pool, void *data)
{
	free(data);
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	muggle_array_list_t array_list;
	muggle_array_list_init(&array_list, 8);

	for (int i = 0; i < 16; i++)
	{
		user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
		data->value = i * 2;
		if (!muggle_array_list_append(&array_list, -1, data))
		{
			LOG_ERROR("failed append data: %d", data->value);
		}
	}

	for (int i = 0; i < muggle_array_list_size(&array_list); i++)
	{
		muggle_array_list_node_t *node = muggle_array_list_index(&array_list, i);
		user_data_t *data = (user_data_t*)node->data;
		LOG_INFO("#%d: %d", i, data->value);
	}

	muggle_array_list_destroy(&array_list, do_free, NULL);

	return 0;
}