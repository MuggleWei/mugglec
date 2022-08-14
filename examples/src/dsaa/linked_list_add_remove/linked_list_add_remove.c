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

	muggle_linked_list_t linked_list;
	muggle_linked_list_init(&linked_list, 8);

	int i = 0;
	for (i = 0; i < 16; i++)
	{
		user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
		data->value = i * 2;
		if (!muggle_linked_list_append(&linked_list, NULL, data))
		{
			LOG_ERROR("failed append data: %d", data->value);
		}
	}

	i = 0;
	muggle_linked_list_node_t *node = muggle_linked_list_first(&linked_list);
	for (; node; node = muggle_linked_list_next(&linked_list, node))
	{
		user_data_t *data = (user_data_t*)node->data;
		LOG_INFO("#%d: %d", i++, data->value);
	}

	node = muggle_linked_list_first(&linked_list);
	while (node)
	{
		node = muggle_linked_list_remove(&linked_list, node, do_free, NULL);
	}

	muggle_linked_list_destroy(&linked_list, NULL, NULL);

	return 0;
}