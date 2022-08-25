#include "muggle/c/muggle_c.h"

typedef struct thread_args
{
	muggle_atomic_int *x;
	muggle_atomic_int *y;
} thread_args_t;

muggle_thread_ret_t func_store(void *p_args)
{
	thread_args_t *args = (thread_args_t*)p_args;
	muggle_atomic_store(args->x, 1, muggle_memory_order_relaxed);
	muggle_atomic_store(args->y, 1, muggle_memory_order_release);

	return 0;
}

muggle_thread_ret_t func_load(void *p_args)
{
	thread_args_t *args = (thread_args_t*)p_args;
	int x = 0, y = 0;
	while (!(y = muggle_atomic_load(args->y, muggle_memory_order_acquire)));
	x = muggle_atomic_load(args->x, muggle_memory_order_relaxed);
	MUGGLE_ASSERT(x != 0 && y != 0);
	LOG_INFO("x=%d, y=%d", x, y);

	return 0;
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	muggle_atomic_int x = 0, y = 0;

	thread_args_t args;
	memset(&args, 0, sizeof(args));
	args.x = &x;
	args.y = &y;

	muggle_thread_t threads[2];
	muggle_thread_create(&threads[0], func_load, &args);
	muggle_thread_create(&threads[1], func_store, &args);

	for (int i = 0; i < 2; i++)
	{
		muggle_thread_join(&threads[i]);
	}

	return 0;
}