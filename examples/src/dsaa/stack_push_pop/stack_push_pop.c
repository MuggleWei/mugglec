#include "muggle/c/muggle_c.h"

typedef struct user_data
{
	int value;
} user_data_t;

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	muggle_stack_t stack;
	muggle_stack_init(&stack, 8);

	for (int i = 0; i < 16; i++)
	{
		user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
		data->value = i * 2;
		if (!muggle_stack_push(&stack, data))
		{
			LOG_ERROR("failed push data: %d", data->value);
		}
	}

	int i = 0;
	muggle_stack_node_t *node = NULL;
	while (!muggle_stack_is_empty(&stack))
	{
		node = muggle_stack_top(&stack);
		user_data_t *data = (user_data_t*)node->data;
		muggle_stack_pop(&stack, NULL, NULL);

		LOG_INFO("#%d: %d", i++, data->value);
		free(data);
	}

	muggle_stack_destroy(&stack, NULL, NULL);

	return 0;
}