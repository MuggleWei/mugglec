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

	muggle_linked_list_t linked_list;
	muggle_linked_list_init(&linked_list, 8);

	int i = 0;
	for (i = 0; i < 16; i++)
	{
		user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
		data->value = i % 2;
		if (!muggle_linked_list_append(&linked_list, NULL, data))
		{
			LOG_ERROR("failed append data: %d", data->value);
		}
	}

	int v = 1;
	muggle_linked_list_node_t *node = NULL;
	while ((node = muggle_linked_list_find(&linked_list, node, &v, cmp_int)) != NULL)
	{
		LOG_INFO("find value=%d at node address 0x%llx", v, (uintptr_t)node);

		if ((node = muggle_linked_list_next(&linked_list, node)) == NULL)
		{
			break;
		}
	};

	muggle_linked_list_destroy(&linked_list, do_free, NULL);

	return 0;
}