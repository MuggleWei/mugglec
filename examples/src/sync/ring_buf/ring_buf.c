#include "muggle/c/muggle_c.h"

typedef struct thread_args
{
	int id;
	muggle_ring_buffer_t *ring;
} thread_args_t;

typedef struct block_data
{
	int block_id;
} block_data_t;

muggle_thread_ret_t consumer(void *p_args)
{
	thread_args_t *args = (thread_args_t*)p_args;

	muggle_ring_buffer_t *ring = args->ring;
	int rpos = 0;
	int cnt = 0;
	while (1)
	{
		block_data_t *data = (block_data_t*)muggle_ring_buffer_read(ring, rpos++);
		if (data->block_id == -1)
		{
			break;
		}

		LOG_INFO("consumer[%d] recv data %d", args->id, data->block_id);
		++cnt;
	}

	LOG_INFO("consumer[%d] exit, total recv: %d", args->id, cnt);

	return 0;
}

#define CONSUMER_NUM 4

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	muggle_ring_buffer_t ring;
	muggle_ring_buffer_init(&ring, 16, 0);

	muggle_thread_t threads[CONSUMER_NUM];
	thread_args_t args[CONSUMER_NUM];
	for (int i = 0; i < CONSUMER_NUM; i++)
	{
		memset(&args[i], 0, sizeof(args[0]));
		args[i].id = i;
		args[i].ring = &ring;
		muggle_thread_create(&threads[i], consumer, &args[i]);
	}

	block_data_t datas[16];
	const int len = sizeof(datas) / sizeof(datas[0]);
	int i = 0;
	for (i = 0; i < 32; i++)
	{
		int idx = i % len;
		datas[idx].block_id = i;
		muggle_ring_buffer_write(&ring, &datas[idx]);
		muggle_msleep(5);
	}

	int idx = i % len;
	datas[idx].block_id = -1;
	muggle_ring_buffer_write(&ring, &datas[idx]);

	for (int i = 0; i < CONSUMER_NUM; i++)
	{
		muggle_thread_join(&threads[i]);
	}

	muggle_ring_buffer_destroy(&ring);

	return 0;
}