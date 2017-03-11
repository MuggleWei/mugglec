#include "muggle/base_c/base_c.h"
#include "muggle/base_c/semaphore.h"

#define THREAD_NUM 8

MuggleSemaphore sem;
MuggleMutexLock lock;
int g_cnt = 0;

THREAD_ROUTINE_RETURN ThreadFunc(void *args)
{
	int thread_idx = *(int*)args, cnt;
	MugglePostSemaphore(&sem, 1);

	// some work to do
	MSleep(50);

	MuggleLockMutexLock(&lock);
	cnt = ++g_cnt;
	MuggleUnlockMutexLock(&lock);

	MUGGLE_INFO("thread id: %d, current number: %d\n", thread_idx, g_cnt);

	return 0;
}

int main(int argc, char *argv[])
{
	MuggleThread th[THREAD_NUM];
	int i;
	g_cnt = 0;

	int attrs = MUGGLE_PERM_USER_READ | MUGGLE_PERM_USER_WRITE;
	if (!MuggleOpenSemaphore(&sem, NULL, 0, attrs, 0))
	{
		MUGGLE_ERROR("Failed open semaphore\n");
		exit(EXIT_FAILURE);
	}

	if (!MuggleInitMutexLock(&lock))
	{
		MUGGLE_ERROR("Failed intiailize mutex lock\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < THREAD_NUM; ++i)
	{
		MuggleThreadCreate(&th[i], NULL, ThreadFunc, &i);
		MuggleWaitSemaphore(&sem);
	}

	for (i = 0; i < THREAD_NUM; ++i)
	{
		MuggleThreadWaitExit(&th[i]);
	}

	MuggleDestroyMutexLock(&lock);
	MuggleDestroySemaphore(&sem);
	
	return 0;
}