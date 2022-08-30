#include "muggle/c/muggle_c.h"

typedef struct task
{
	int idx;
	int priority;
} task_t;

int priority_cmp(const void *d1, const void *d2)
{
	task_t *t1 = (task_t*)d1;
	task_t *t2 = (task_t*)d2;
	return t1->priority - t2->priority;
}

void do_free(void *pool, void *data)
{
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
		task_t *task = (task_t*)malloc(sizeof(task_t));
		task->idx = i;
		task->priority = 1 + rand() % 8;
		LOG_INFO("generate task: idx=%d, priority=%d", task->idx, task->priority);
		if (!muggle_heap_insert(&heap, task, NULL))
		{
			LOG_ERROR("failed add task: %d", i);
		}
	}

	muggle_heap_node_t *node = NULL;
	while ((node = muggle_heap_root(&heap)) != NULL)
	{
		task_t *task = (task_t*)node->key;
		LOG_INFO("highest priority task: idx=%d, priority=%d", task->idx, task->priority);
		muggle_heap_remove(&heap, node, do_free, NULL, NULL, NULL);
	}

	muggle_heap_destroy(&heap, NULL, NULL, NULL, NULL);

	return 0;
}