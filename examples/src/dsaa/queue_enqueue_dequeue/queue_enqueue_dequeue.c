#include "muggle/c/muggle_c.h"

typedef struct user_data
{
	int value;
} user_data_t;

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	muggle_queue_t queue;
	muggle_queue_init(&queue, 8);

	for (int i = 0; i < 16; i++)
	{
		user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
		data->value = i * 2;
		if (!muggle_queue_enqueue(&queue, data))
		{
			LOG_ERROR("failed enqueue data: %d", data->value);
		}
	}

	int i = 0;
	muggle_queue_node_t *node = NULL;
	while (!muggle_queue_is_empty(&queue))
	{
		node = muggle_queue_front(&queue);
		user_data_t *data = (user_data_t*)node->data;
		muggle_queue_dequeue(&queue, NULL, NULL);
		
		LOG_INFO("#%d: %d", i++, data->value);
		free(data);
	}

	muggle_queue_destroy(&queue, NULL, NULL);

	return 0;
}