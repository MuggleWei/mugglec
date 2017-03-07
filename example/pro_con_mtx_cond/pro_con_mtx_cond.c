#include "muggle/base_c/base_c.h"

typedef struct ProduceThread
{
	MuggleThread tid;
	int idx;
	int task_cnt;
}ThreadWithTask;

volatile int cur_prod = 0;
MuggleMutexLock mtx;
MuggleCondVar cond;

THREAD_ROUTINE_RETURN ProducerRoutine(void *args)
{
	ThreadWithTask *p = (ThreadWithTask*)args;

	int i = 0;
	for (i = 0; i < p->task_cnt; ++i)
	{
		if (!MuggleLockMutexLock(&mtx))
		{
			MUGGLE_ERROR("Failed lock mutex lock in producer: %d\n", p->idx);
			exit(EXIT_FAILURE);
		}

		++cur_prod;

		if (!MuggleUnlockMutexLock(&mtx))
		{
			MUGGLE_ERROR("Failed unlock mutex lock in producer: %d\n", p->idx);
			exit(EXIT_FAILURE);
		}

		if (!MuggleWakeCondVar(&cond))
		{
			MUGGLE_ERROR("Failed wake condition variable: %d\n", p->idx);
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int tasks[] = { 5000, 5000, 5000, 5000, 6000, 8000, 9000 };
	int cnt = (int)(sizeof(tasks) / sizeof(tasks[0]));
	int i, total = 0, total_consume = 0;
	bool ret;

	ThreadWithTask *producer_args = (ThreadWithTask*)malloc(sizeof(ThreadWithTask) * cnt);
	if (producer_args == NULL)
	{
		MUGGLE_ERROR("Failed create thread tasks array\n");
		exit(EXIT_FAILURE);
	}

	ret = MuggleInitMutexLock(&mtx);
	if (!ret)
	{
		MUGGLE_ERROR("Failed init mutex lock\n");
		exit(EXIT_FAILURE);
	}

	ret = MuggleInitCondVar(&cond);
	if (!ret)
	{
		MUGGLE_ERROR("Failed init condition variable\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < cnt; ++i)
	{
		producer_args[i].idx = i;
		producer_args[i].task_cnt = tasks[i];
		total += tasks[i];
		MuggleThreadCreate(&producer_args[i].tid, NULL, ProducerRoutine, &producer_args[i]);
	}

	while (true)
	{
		if (total_consume == total)
		{
			break;
		}

		if (!MuggleLockMutexLock(&mtx))
		{
			MUGGLE_ERROR("Failed lock mutex lock\n");
			exit(EXIT_FAILURE);
		}

		while (cur_prod == 0)
		{
			if (!MuggleWaitCondVar(&cond, &mtx, -1))
			{
				MUGGLE_ERROR("Failed wait condition variable\n");
				exit(EXIT_FAILURE);
			}
		}		

		while (cur_prod > 0)
		{
			--cur_prod;
			++total_consume;
		}

		if (!MuggleUnlockMutexLock(&mtx))
		{
			MUGGLE_ERROR("Failed unlock mutex lock\n");
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < cnt; ++i)
	{
		if (!MuggleThreadWaitExit(&producer_args[i].tid))
		{
			MUGGLE_ERROR("Failed in thread exit: %s\n", i);
			exit(EXIT_FAILURE);
		}
	}
	MUGGLE_ASSERT_MSG(cur_prod == 0, "Something wrong\n");

	ret = MuggleDestroyCondVar(&cond);
	if (!ret)
	{
		MUGGLE_ERROR("Failed destroy condition variable\n");
		exit(EXIT_FAILURE);
	}

	ret = MuggleDestroyMutexLock(&mtx);
	if (!ret)
	{
		MUGGLE_ERROR("Failed destroy mutex lock\n");
		exit(EXIT_FAILURE);
	}

	MUGGLE_INFO("Total consume: %d\n", total_consume);

	return 0;
}