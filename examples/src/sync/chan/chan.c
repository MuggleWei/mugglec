#include "muggle/c/muggle_c.h"

typedef struct thread_args
{
	int id;
	int target_cnt;
	muggle_channel_t *chan;
} thread_args_t;

typedef struct block_data
{
	int thread_id;
	int block_id;
} block_data_t;

muggle_thread_ret_t producer(void *p_args)
{
	thread_args_t *args = (thread_args_t*)p_args;

	muggle_channel_t *chan = args->chan;
	for (int i = 0; i < args->target_cnt; i++)
	{
		block_data_t *data = (block_data_t*)malloc(sizeof(block_data_t));
		data->thread_id = args->id;
		data->block_id = i;

		if (muggle_channel_write(chan, data) == MUGGLE_ERR_FULL)
		{
			LOG_WARNING("failed write #%d[%d], channel full!", data->block_id, data->thread_id);
			free(data);
		}
	}

	block_data_t *data = (block_data_t*)malloc(sizeof(block_data_t));
	data->thread_id = args->id;
	data->block_id = -1;
	while (muggle_channel_write(chan, (void*)data) == MUGGLE_ERR_FULL);

	return 0;
}

#define PRODUCER_NUM 4

int main(int argc, char const *argv[])
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	muggle_channel_t chan;
	muggle_channel_init(&chan, 16, 0);

	muggle_thread_t threads[PRODUCER_NUM];
	thread_args_t args[PRODUCER_NUM];
	for (int i = 0; i < PRODUCER_NUM; i++)
	{
		memset(&args[i], 0, sizeof(args[0]));
		args[i].id = i;
		args[i].target_cnt = 8;
		args[i].chan = &chan;
		muggle_thread_create(&threads[i], producer, &args[i]);
	}

	int completed_cnt = 0;
	while (1)
	{
		block_data_t *data = muggle_channel_read(&chan);
		if (data->block_id == -1)
		{
			LOG_INFO("recv [%d] exit data", data->thread_id);
			completed_cnt++;
			if (completed_cnt == PRODUCER_NUM)
			{
				break;
			}
		}
		else
		{
			LOG_INFO("consume #%d[%d]", data->block_id, data->thread_id);
			free(data);
		}
	}

	for (int i = 0; i < PRODUCER_NUM; i++)
	{
		muggle_thread_join(&threads[i]);
	}

	muggle_channel_destroy(&chan);

	return 0;
}
