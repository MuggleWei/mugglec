#include "muggle/c/muggle_c.h"

typedef struct foo
{
	int32_t idx;
	int32_t val;
} foo_t;

typedef struct thread_args
{
	int32_t idx;
	muggle_channel_t *chan;
	muggle_sowr_memory_pool_t *pool;
} thread_args_t;

muggle_thread_ret_t producer(void *p_args)
{
	thread_args_t *args = (thread_args_t*)p_args;
	muggle_channel_t *chan = args->chan;
	muggle_sowr_memory_pool_t *pool = args->pool;

	for (int i = 0; i < 1024; i++)
	{
		foo_t *foo = muggle_sowr_memory_pool_alloc(pool);
		if (foo == NULL)
		{
			continue;
		}
		foo->idx = args->idx;
		foo->val = i;

		muggle_channel_write(chan, foo);
	}

	// send exit signal
	while (muggle_channel_write(chan, NULL) != 0);

	LOG_INFO("producer[%d] exit", args->idx);

	return 0;
}

#define NUM_PRODUCER 1

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	// initiailize channel
	muggle_channel_t chan;
	muggle_channel_init(&chan, 512, 0);

	// initiailize memory pool
	muggle_sowr_memory_pool_t pool;
	muggle_sowr_memory_pool_init(&pool, 512, sizeof(foo_t));

	// create producer threads
	muggle_thread_t threads[NUM_PRODUCER];
	thread_args_t args[NUM_PRODUCER];
	for (int i = 0; i < (int)(sizeof(threads)/sizeof(threads[0])); i++)
	{
		args[i].idx = i;
		args[i].chan = &chan;
		args[i].pool = &pool;
		muggle_thread_create(&threads[i], producer, &args[i]);
	}

	// consumer
	int cnt = 0;
	int exit_signal = 0;
	while (1)
	{
		foo_t *foo = (foo_t*)muggle_channel_read(&chan);
		if (foo)
		{
			muggle_sowr_memory_pool_free(foo);
			cnt++;
		}
		else
		{
			exit_signal++;
			LOG_INFO("recv exit signal, total: %d", exit_signal);
			if (exit_signal == NUM_PRODUCER)
			{
				break;
			}
		}
		
	}
	LOG_INFO("consumer total recv message: %d", cnt);

	// join threads
	for (int i = 0; i < (int)(sizeof(threads)/sizeof(threads[0])); i++)
	{
		muggle_thread_join(&threads[i]);
	}

	// destroy memory pool
	muggle_sowr_memory_pool_destroy(&pool);

	// destroy channel
	muggle_channel_destroy(&chan);

	return 0;
}