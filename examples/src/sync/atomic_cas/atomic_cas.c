#include "muggle/c/muggle_c.h"

typedef struct thread_args
{
	muggle_atomic_int *val;
	int       id;
	int       cnt;
} thread_args_t;

int ref_cnt_release(muggle_atomic_int *val)
{
	muggle_atomic_int v = 0, desired=0;
	do {
		v = *val;
		if (v == 0)
		{
			return -1;
		}
		desired = v - 1;
	} while (!muggle_atomic_cmp_exch_strong(val, &v, desired, muggle_memory_order_relaxed));

	return desired;
}

muggle_thread_ret_t ref_release_routine(void *p_args)
{
	thread_args_t *args = (thread_args_t*)p_args;

	while (1)
	{
		int v = ref_cnt_release(args->val);
		if (v < 0)
		{
			break;
		}

		args->cnt++;
	}

	LOG_INFO("release count: %d", args->cnt);

	return 0;
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	int init_val = 1024 * 1024;
	muggle_atomic_int val = init_val;

	muggle_thread_t threads[4];
	thread_args_t args[4];
	memset(args, 0, sizeof(args));
	for (int i = 0; i < 4; i++)
	{
		args[i].val = &val;
		args[i].id = i;
		muggle_thread_create(&threads[i], ref_release_routine, &args[i]);
	}

	int total_cnt = 0;
	for (int i = 0; i < 4; i++)
	{
		muggle_thread_join(&threads[i]);
		total_cnt += args[i].cnt;
	}

	LOG_INFO("total count: %d", total_cnt);
	MUGGLE_ASSERT(total_cnt == init_val);

	return 0;
}