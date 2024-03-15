#include "muggle/c/muggle_c.h"

typedef struct foo {
	int32_t val;
} foo_t;

typedef struct thread_args {
	int32_t idx;
	muggle_channel_t *chan;
} thread_args_t;

muggle_thread_ret_t consumer(void *p_args)
{
	thread_args_t *args = (thread_args_t *)p_args;
	muggle_channel_t *chan = args->chan;

	int cnt = 0;
	while (1) {
		foo_t *foo = (foo_t *)muggle_channel_read(chan);
		if (foo) {
			muggle_ring_memory_pool_free(foo);
			cnt++;
		} else {
			LOG_INFO("recv exit signal, #%d total recv: %d", args->idx, cnt);
			break;
		}
	}

	return 0;
}

#define NUM_CONSUMER 4

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	// initiailize memory pool
	muggle_ring_memory_pool_t pool;
	muggle_ring_memory_pool_init(&pool, 512, sizeof(foo_t));

	// create consumer threads
	muggle_thread_t threads[NUM_CONSUMER];
	muggle_channel_t chans[NUM_CONSUMER];
	thread_args_t args[NUM_CONSUMER];
	for (int i = 0; i < (int)(sizeof(threads) / sizeof(threads[0])); i++) {
		muggle_channel_init(&chans[i], 512, 0);

		args[i].idx = i;
		args[i].chan = &chans[i];
		muggle_thread_create(&threads[i], consumer, &args[i]);
	}

	// producer
	for (int i = 0; i < 1024; i++) {
		foo_t *foo = muggle_ring_memory_pool_alloc(&pool);
		foo->val = i;

		int idx_consumer = i % NUM_CONSUMER;
		muggle_channel_write(&chans[idx_consumer], foo);
	}

	for (int i = 0; i < NUM_CONSUMER; i++) {
		muggle_channel_write(&chans[i], NULL);
	}

	// join threads
	for (int i = 0; i < (int)(sizeof(threads) / sizeof(threads[0])); i++) {
		muggle_thread_join(&threads[i]);
		muggle_channel_destroy(&chans[i]);
	}

	// destroy memory pool
	muggle_ring_memory_pool_destroy(&pool);

	return 0;
}
