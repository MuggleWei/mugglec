/******************************************************************************
 *  @file         shm.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2025-01-17
 *  @copyright    Copyright 2025 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec shared memory
 *****************************************************************************/

#ifndef MUGGLE_C_SHM_H_
#define MUGGLE_C_SHM_H_

#include "muggle/c/base/macro.h"
#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#endif

#include <stdint.h>

EXTERN_C_BEGIN

enum {
	MUGGLE_SHM_FLAG_OPEN = 0x00,
	MUGGLE_SHM_FLAG_CREAT = 0x01,
};

enum {
	MUGGLE_SHM_FLAG_PRIVILEGE_USER = 0x00 << 8,
	MUGGLE_SHM_FLAG_PRIVILEGE_GROUP = 0x01 << 8,
	MUGGLE_SHM_FLAG_PRIVILEGE_OTHER = 0x02 << 8,
};

typedef struct {
	void *ptr; //!< pointer to shared memory
#if MUGGLE_PLATFORM_WINDOWS
	HANDLE hMapFile;
#else
	int shm_id; //!< shared memory id
#endif
	uint32_t nbytes; //!< number of bytes (user input, possible 0)
} muggle_shm_t;

/**
 * @brief open shared memory
 *
 * @param shm     pointer to muggle_shm_t
 * @param k_name  key's name of shm, in *nix, gurantee it's an exists filepath
 * @param k_num   key's number, range in [1, 255]
 * @param flag    bit or of MUGGLE_SHM_FLAG_*
 * @param nbytes  expected size of memory
 *
 * @return
 *   - on success, return pointer to memory
 *   - on failed, return NULL
 *
 * @NOTE
 *   - if init_flag is MUGGLE_SHM_INIT_FLAG_OPEN, will open existing shared 
 *     memory, if it's not exists, return NULL
 *   - if init_flag is MUGGLE_SHM_INIT_FLAG_CREAT, will create an new shared 
 *     memory, if it's already exists, return NULL
 *   - if init_flag  is MUGGLE_SHM_INIT_FLAG_OPEN and nbytes is 0, will mapping 
 *     entire shared memory segment
 */
MUGGLE_C_EXPORT
void *muggle_shm_open(muggle_shm_t *shm, const char *k_name, int k_num,
					  int flag, uint32_t nbytes);

/**
 * @brief detach shared memory
 *
 * @param shm  pointer to muggle_shm_t
 *
 * @return
 *   - on success, return 0
 *   - otherwise failed
 */
MUGGLE_C_EXPORT
int muggle_shm_detach(muggle_shm_t *shm);

/**
 * @brief remove shared memory
 *
 * @param shm  pointer to muggle_shm_t
 *
 * @return
 *   - on success, return 0
 *   - otherwise failed
 */
MUGGLE_C_EXPORT
int muggle_shm_rm(muggle_shm_t *shm);

EXTERN_C_END

#endif // !MUGGLE_C_SHM_H_
