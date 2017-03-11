#include "muggle/base_c/semaphore.h"
#include "muggle/base_c/log.h"

#if MUGGLE_PLATFORM_WINDOWS

#define MAX_NUM_WINDOWS_SEMAPHORE 512

bool MuggleOpenSemaphore(MuggleSemaphore *sem, const char *name, int flags, int attr, int init_val)
{
	MUGGLE_ASSERT_MSG(sem != NULL, "Semaphore pointer is NULL\n");
	if (sem == NULL)
	{
		return false;
	}
	
	sem->sem = CreateSemaphore(NULL, (LONG)init_val, MAX_NUM_WINDOWS_SEMAPHORE, NULL);

	return sem->sem != NULL;
}
bool MugglePostSemaphore(MuggleSemaphore *sem, int add_cnt)
{
	MUGGLE_ASSERT_MSG(sem != NULL, "Semaphore pointer is NULL\n");
	if (sem == NULL)
	{
		return false;
	}

	if (add_cnt <= 0)
	{
		return false;
	}

	return ReleaseSemaphore(sem->sem, add_cnt, NULL);
}
bool MuggleWaitSemaphore(MuggleSemaphore *sem)
{
	MUGGLE_ASSERT_MSG(sem != NULL, "Semaphore pointer is NULL\n");
	if (sem == NULL)
	{
		return false;
	}

	return WaitForSingleObject(sem->sem, INFINITE) == WAIT_OBJECT_0;
}
bool MuggleDestroySemaphore(MuggleSemaphore *sem)
{
	MUGGLE_ASSERT_MSG(sem != NULL, "Semaphore pointer is NULL\n");
	if (sem == NULL)
	{
		return false;
	}

	return CloseHandle(sem->sem);
}

#else

#include <fcntl.h>

union semun
{
	int 			val;		// value for SETVAL
	struct semid_ds *buf;		// Buffer for IPC_STAT, IPC_SET
	unsigned short 	*array;		// Array for GETALL, SETALL
#if MUGGLE_PLATFORM_LINUX
	struct seminfo	*__buf;		// buffer for IPC_INFO
#endif
};

bool MuggleOpenSemaphore(MuggleSemaphore *sem, const char *name, int flags, int attr, int init_val)
{
	mode_t mode;
	union semun arg;

	MUGGLE_ASSERT_MSG(sem != NULL, "Semaphore pointer is NULL\n");
	if (sem == NULL)
	{
		return false;
	}

	mode =
		((attr & MUGGLE_PERM_USER_READ) ? S_IRUSR : 0) |
		((attr & MUGGLE_PERM_USER_WRITE) ? S_IWUSR : 0) |
		((attr & MUGGLE_PERM_USER_EXECUTE) ? S_IXUSR : 0) |
		((attr & MUGGLE_PERM_GRP_READ) ? S_IRGRP : 0) |
		((attr & MUGGLE_PERM_GRP_WRITE) ? S_IWGRP : 0) |
		((attr & MUGGLE_PERM_GRP_EXECUTE) ? S_IXGRP : 0) |
		((attr & MUGGLE_PERM_OTHER_READ) ? S_IROTH : 0) |
		((attr & MUGGLE_PERM_OTHER_WRITE) ? S_IWOTH : 0) |
		((attr & MUGGLE_PERM_OTHER_EXECUTE) ? S_IXOTH : 0);

	if (name == NULL)
	{
		sem->sem = semget(IPC_PRIVATE, 1, mode);
	}
	else
	{
		int addition_flags =
			((flags & MUGGLE_SEMAPHORE_CREAT) ? O_CREAT : 0) |
			((flags & MUGGLE_SEMAPHORE_EXCL) ? O_EXCL : 0);

		key_t key = ftok(name, 0);
		if (key == -1)
		{
			return false;
		}

		sem->sem = semget(key, 1, addition_flags | mode);
	}

	if (sem->sem == -1)
	{
		return false;
	}

	arg.val = 0;
	if (semctl(sem->sem, 0, SETVAL, arg) == -1)
	{
		semctl(sem->sem, 0, IPC_RMID, 0);
		return false;
	}

	return true;
}
bool MugglePostSemaphore(MuggleSemaphore *sem, int add_cnt)
{
	struct sembuf sop;

	MUGGLE_ASSERT_MSG(sem != NULL, "Semaphore pointer is NULL\n");
	if (sem == NULL)
	{
		return false;
	}

	if (add_cnt <= 0)
	{
		return false;
	}

	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;

	return semop(sem->sem, &sop, 1) == 0;
}
bool MuggleWaitSemaphore(MuggleSemaphore *sem)
{
	struct sembuf sop;

	MUGGLE_ASSERT_MSG(sem != NULL, "Semaphore pointer is NULL\n");
	if (sem == NULL)
	{
		return false;
	}

	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = 0;

	return semop(sem->sem, &sop, 1) == 0;
}
bool MuggleDestroySemaphore(MuggleSemaphore *sem)
{
	MUGGLE_ASSERT_MSG(sem != NULL, "Semaphore pointer is NULL\n");
	if (sem == NULL)
	{
		return false;
	}

	if (semctl(sem->sem, 0, IPC_RMID, 0) == -1)
	{
		return false;
	}

	return true;
}

#endif