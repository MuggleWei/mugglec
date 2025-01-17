#include "shm.h"
#include <string.h>
#if MUGGLE_PLATFORM_WINDOWS

void *muggle_shm_open(muggle_shm_t *shm, const char *k_name, int k_num,
					  int flag, uint32_t nbytes)
{
	// TODO: to be continued ......
	return -1;
}

int muggle_shm_detach(muggle_shm_t *shm)
{
	// TODO: to be continued ......
	return -1;
}

int muggle_shm_rm(muggle_shm_t *shm)
{
	// TODO: to be continued ......
	return -1;
}

#else
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <unistd.h>

void *muggle_shm_open(muggle_shm_t *shm, const char *k_name, int k_num,
					  int flag, uint32_t nbytes)
{
	memset(shm, 0, sizeof(*shm));

	// get shm key
	key_t shm_key = ftok(k_name, k_num);
	if (shm_key == -1) {
		return NULL;
	}

	// get shm id or create a new shm
	int flag_open = 0;
	int flag_privilege = SHM_R | SHM_W;

	if (flag & MUGGLE_SHM_FLAG_CREAT) {
		flag_open = IPC_CREAT | IPC_EXCL;
	}
	if (flag & MUGGLE_SHM_FLAG_PRIVILEGE_GROUP) {
		flag_privilege |= 0060;
	}
	if (flag & MUGGLE_SHM_FLAG_PRIVILEGE_OTHER) {
		flag_privilege |= 0006;
	}

	int shm_id = shmget(shm_key, nbytes, flag_open | flag_privilege);
	if (shm_id == -1) {
		return NULL;
	}

	// shm attach
	void *ptr = (void *)shmat(shm_id, NULL, flag_privilege);
	if (ptr == (void *)(-1)) {
		return NULL;
	}

	shm->ptr = ptr;
	shm->shm_id = shm_id;
	shm->nbytes = nbytes;

	return shm->ptr;
}

int muggle_shm_detach(muggle_shm_t *shm)
{
	if (shm->ptr == NULL) {
		return -1;
	}

	if (shmdt(shm->ptr) != 0) {
		return -1;
	}

	shm->ptr = NULL;

	return 0;
}

int muggle_shm_rm(muggle_shm_t *shm)
{
	if (shmctl(shm->shm_id, IPC_RMID, NULL) == -1) {
		return -1;
	}
	return 0;
}

#endif
