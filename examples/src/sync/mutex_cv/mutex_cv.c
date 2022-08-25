#include "muggle/c/muggle_c.h"

typedef struct thread_args
{
	int id;

	int *arr;
	int arr_size;

	int *pos;
	muggle_mutex_t *mtx;
	muggle_condition_variable_t *cv;
} thread_args_t;

muggle_thread_ret_t producer(void *p_args)
{
	thread_args_t *args = (thread_args_t*)p_args;
	int *arr = args->arr;
	int *pos = args->pos;

	while (1)
	{
		muggle_mutex_lock(args->mtx);

		if (*pos == args->arr_size)
		{
			muggle_mutex_unlock(args->mtx);
			break;
		}

		arr[(*pos)++] = args->id;

		muggle_mutex_unlock(args->mtx);
		muggle_condition_variable_notify_one(args->cv);

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

	int pos = 0;
	muggle_mutex_t mtx;
	muggle_mutex_init(&mtx);

	muggle_condition_variable_t cv;
	muggle_condition_variable_init(&cv);

	muggle_thread_t threads[PRODUCER_NUM];
	thread_args_t args[PRODUCER_NUM];
	for (int i = 0; i < PRODUCER_NUM; i++)
	{
		memset(&args[i], 0, sizeof(args[0]));
		args[i].id = i;
		args[i].arr = arr;
		args[i].arr_size = ARRAY_SIZE;
		args[i].pos = &pos;
		args[i].mtx = &mtx;
		args[i].cv = &cv;
		muggle_thread_create(&threads[i], producer, &args[i]);
	}

	int r_pos = 0;
	muggle_mutex_lock(&mtx);
	while (1)
	{
		if (r_pos == pos)
		{
			if (pos == ARRAY_SIZE)
			{
				muggle_mutex_unlock(&mtx);
				break;
			}
			muggle_condition_variable_wait(&cv, &mtx, NULL);
		}
		else
		{
			for (; r_pos < pos; r_pos++)
			{
				LOG_INFO("consume #%d[%d]", r_pos, arr[r_pos]);
			}
		}
	}
	muggle_mutex_unlock(&mtx);

	for (int i = 0; i < PRODUCER_NUM; i++)
	{
		muggle_thread_join(&threads[i]);
	}

	muggle_condition_variable_destroy(&cv);
	muggle_mutex_destroy(&mtx);

	return 0;
}