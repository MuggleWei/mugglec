#include "muggle/c/muggle_c.h"

typedef struct thread_args
{
	int id;

	int *arr;
	int arr_size;

	muggle_atomic_int *pos;
	muggle_atomic_int *spin;
} thread_args_t;

muggle_thread_ret_t producer(void *p_args)
{
	thread_args_t *args = (thread_args_t*)p_args;
	int *arr = args->arr;
	int *pos = args->pos;

	while (1)
	{
		muggle_spinlock_lock(args->spin);

		if (*pos == args->arr_size)
		{
			muggle_spinlock_unlock(args->spin);
			break;
		}
		arr[(*pos)++] = args->id;

		muggle_spinlock_unlock(args->spin);

		muggle_msleep(5);
	}

	return 0;
}

#define ARRAY_SIZE 16
#define PRODUCER_NUM 4

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	int arr[ARRAY_SIZE];

	muggle_atomic_int pos = 0;

	muggle_atomic_int spin;
	muggle_spinlock_init(&spin);

	muggle_thread_t threads[PRODUCER_NUM];
	thread_args_t args[PRODUCER_NUM];
	for (int i = 0; i < PRODUCER_NUM; i++)
	{
		memset(&args[i], 0, sizeof(args[0]));
		args[i].id = i;
		args[i].arr = arr;
		args[i].arr_size = ARRAY_SIZE;
		args[i].pos = &pos;
		args[i].spin = &spin;
		muggle_thread_create(&threads[i], producer, &args[i]);
	}

	int r_pos = 0;
	while (1)
	{
		muggle_spinlock_lock(&spin);

		if (r_pos == pos)
		{
			if (pos == ARRAY_SIZE)
			{
				muggle_spinlock_unlock(&spin);
				break;
			}
		}
		else
		{
			for (; r_pos < pos; r_pos++)
			{
				LOG_INFO("consume #%d[%d]", r_pos, arr[r_pos]);
			}
		}

		muggle_spinlock_unlock(&spin);
	}

	for (int i = 0; i < PRODUCER_NUM; i++)
	{
		muggle_thread_join(&threads[i]);
	}

	return 0;
}