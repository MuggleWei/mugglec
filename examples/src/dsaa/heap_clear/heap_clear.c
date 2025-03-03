#include "muggle/c/muggle_c.h"

typedef struct task
{
	int idx;
	int priority;
} my_task_t;

int priority_cmp(const void *d1, const void *d2)
{
	my_task_t *t1 = (my_task_t*)d1;
	my_task_t *t2 = (my_task_t*)d2;
	return t1->priority - t2->priority;
}

void do_free(void *pool, void *data)
{
	MUGGLE_UNUSED(pool);
	free(data);
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	srand((unsigned int)time(NULL));

	muggle_heap_t heap;
	muggle_heap_init(&heap, priority_cmp, 8);

	for (int i = 0; i < 16; i++)
	{
		my_task_t *task = (my_task_t*)malloc(sizeof(my_task_t));
		task->idx = i;
		task->priority = 1 + rand() % 8;
		LOG_INFO("generate task: idx=%d, priority=%d", task->idx, task->priority);
		if (!muggle_heap_insert(&heap, task, NULL))
		{
			LOG_ERROR("failed add task: %d", i);
		}
	}

	LOG_INFO("clear heap");
	muggle_heap_clear(&heap, do_free, NULL, NULL, NULL);

	muggle_heap_destroy(&heap, NULL, NULL, NULL, NULL);

	return 0;
}
