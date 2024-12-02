#include "muggle/c/muggle_c.h"

typedef struct task
{
	int idx;
	int priority;
} mysql_task_t;

int priority_cmp(const void *d1, const void *d2)
{
	mysql_task_t *t1 = (mysql_task_t*)d1;
	mysql_task_t *t2 = (mysql_task_t*)d2;
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
		mysql_task_t *task = (mysql_task_t*)malloc(sizeof(mysql_task_t));
		task->idx = i;
		task->priority = 1 + rand() % 8;
		LOG_INFO("generate task: idx=%d, priority=%d", task->idx, task->priority);
		if (!muggle_heap_insert(&heap, task, NULL))
		{
			LOG_ERROR("failed add task: %d", i);
		}
	}

	muggle_heap_destroy(&heap, do_free, NULL, NULL, NULL);

	return 0;
}
