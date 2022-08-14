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

	muggle_queue_destroy(&queue, do_free, NULL);

	return 0;
}