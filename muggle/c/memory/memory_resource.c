/******************************************************************************
 *  @file         memory_resource.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2026-09-16
 *  @copyright    Copyright 2026 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec memory resource
 *****************************************************************************/

#include "memory_resource.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "muggle/c/base/utils.h"

#if MUGGLE_PLATFORM_LINUX
	#include <sys/mman.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/ipc.h>
	#include <sys/shm.h>
#elif MUGGLE_PLATFORM_WINDOWS
	#include <memoryapi.h>
#endif

static bool muggle_memory_res_init_default(muggle_memory_resource_t *res,
										   size_t nbytes)
{
	res->flags.mem_type = MUGGLE_MEMORY_RES_TYPE_NULL;

	res->data = malloc(nbytes);
	if (res->data == NULL) {
		return false;
	}
	res->n_bytes = nbytes;

	return true;
}

static void muggle_memory_res_destroy_default(muggle_memory_resource_t *res)
{
	if (res->data) {
		free(res->data);
		res->data = NULL;
	}
}

#if MUGGLE_PLATFORM_LINUX

static bool muggle_memory_res_init_huge_private(muggle_memory_resource_t *res,
												size_t nbytes)
{
	int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB;

	#if MUGGLE_C_HAVE_MAP_HUGE_NBYTES

	switch (res->flags.huge_type) {
	case MUGGLE_MEMORY_RES_HUGE_2MB: {
		flags |= MAP_HUGE_2MB;
	} break;
	case MUGGLE_MEMORY_RES_HUGE_1GB: {
		flags |= MAP_HUGE_1GB;
	} break;
	}

	#endif

	// mmap
	void *datas = mmap(NULL, nbytes, PROT_READ | PROT_WRITE, flags, -1, 0);
	if (datas == MAP_FAILED) {
		return false;
	}

	res->data = datas;
	res->n_bytes = nbytes;

	return true;
}

static void
muggle_memory_res_destroy_huge_private(muggle_memory_resource_t *res)
{
	if (res->data) {
		munmap(res->data, res->n_bytes);
		res->data = NULL;
	}
}

static bool muggle_memory_res_init_huge_share(muggle_memory_resource_t *res,
											  size_t nbytes, const char *k_name,
											  int k_num)
{
	// get shm key
	key_t shm_key = ftok(k_name, k_num);
	if (shm_key == -1) {
		return NULL;
	}

	// get shm id or create a new shm
	int flag_open = 0;
	int flag_privilege = SHM_R | SHM_W;
	int flag_huge = SHM_HUGETLB;

	if (res->flags.share_flag & MUGGLE_MEMORY_RES_SHM_CREATE) {
		flag_open = IPC_CREAT | IPC_EXCL;
	}

	if (res->flags.share_flag & MUGGLE_MEMORY_RES_SHM_PRIVILEGE_GROUP) {
		flag_privilege |= 0060;
	}
	if (res->flags.share_flag & MUGGLE_MEMORY_RES_SHM_PRIVILEGE_OTHER) {
		flag_privilege |= 0006;
	}

	#if MUGGLE_C_HAVE_SHM_HUGE_NBYTES
	switch (res->flags.huge_type) {
	case MUGGLE_MEMORY_RES_HUGE_2MB: {
		flag_huge |= SHM_HUGE_2MB;
	} break;
	case MUGGLE_MEMORY_RES_HUGE_1GB: {
		flag_huge |= SHM_HUGE_1GB;
	} break;
	}
	#endif

	int shm_id =
		shmget(shm_key, nbytes, flag_open | flag_privilege | flag_huge);
	if (shm_id == -1) {
		return false;
	}

	// shm attach
	void *ptr = (void *)shmat(shm_id, NULL, flag_privilege);
	if (ptr == (void *)(-1)) {
		return false;
	}

	res->shm.shm_id = shm_id;
	res->data = ptr;
	res->n_bytes = nbytes;

	return true;
}

static void muggle_memory_res_destroy_huge_share(muggle_memory_resource_t *res)
{
	if (res->data) {
		shmdt(res->data);
		res->data = NULL;
	}
}

#elif MUGGLE_PLATFORM_WINDOWS

BOOL EnableLockMemoryPrivilege()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(),
						  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return FALSE;

	if (!LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &luid)) {
		CloseHandle(hToken);
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	BOOL result =
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
	DWORD err = GetLastError();

	CloseHandle(hToken);
	return (result && err == ERROR_SUCCESS);
}

static bool muggle_memory_res_init_huge_private(muggle_memory_resource_t *res,
												size_t nbytes)
{
	if (!EnableLockMemoryPrivilege()) {
		return false;
	}

	void *datas = NULL;
	switch (res->flags.huge_type) {
	#if MUGGLE_C_HAVE_VIRTUALALLOC2
	case MUGGLE_MEMORY_RES_HUGE_1GB: {
		MEM_EXTENDED_PARAMETER extended = { 0 };
		extended.Type = MemExtendedParameterAttributeFlags;
		extended.ULong64 = MEM_EXTENDED_PARAMETER_NONPAGED_HUGE;
		datas = VirtualAlloc2(GetCurrentProcess(), NULL, nbytes,
							  MEM_LARGE_PAGES | MEM_RESERVE | MEM_COMMIT,
							  PAGE_READWRITE, &extended, 1);

	} break;
	#endif
	default: {
		datas = VirtualAlloc(NULL, nbytes,
							 MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES,
							 PAGE_READWRITE);
	} break;
	}

	if (datas == NULL) {
		return false;
	}

	res->data = datas;
	res->n_bytes = nbytes;

	return true;
}

static void
muggle_memory_res_destroy_huge_private(muggle_memory_resource_t *res)
{
	if (res->data) {
		VirtualFree(res->data, 0, MEM_RELEASE);
		res->data = NULL;
	}
}

static bool muggle_memory_res_init_huge_share(muggle_memory_resource_t *res,
											  size_t nbytes, const char *k_name,
											  int k_num)
{
	if (!EnableLockMemoryPrivilege()) {
		return false;
	}

	if (res->flags.share_flag & MUGGLE_MEMORY_RES_SHM_CREATE) {
		DWORD flags = PAGE_READWRITE | SEC_COMMIT | SEC_LARGE_PAGES;
		LARGE_INTEGER li;
		li.QuadPart = nbytes;
		res->shm.hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL,
											   flags, (DWORD)li.HighPart,
											   (DWORD)li.LowPart, k_name);
	} else {
		res->shm.hMapFile =
			OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, k_name);
	}

	if (res->shm.hMapFile == NULL) {
		return false;
	}

	res->data = MapViewOfFile(
		res->shm.hMapFile, FILE_MAP_ALL_ACCESS | FILE_MAP_LARGE_PAGES, 0, 0, 0);
	if (res->data == NULL) {
		CloseHandle(res->shm.hMapFile);
		return false;
	}
	res->n_bytes = nbytes;

	return true;
}

static void muggle_memory_res_destroy_huge_share(muggle_memory_resource_t *res)
{
	if (res->data) {
		UnmapViewOfFile(res->data);
		res->data = NULL;

		CloseHandle(res->shm.hMapFile);
	}
}

#else

static bool muggle_memory_res_init_huge_private(muggle_memory_resource_t *res,
												size_t nbytes)
{
	MUGGLE_UNUSED(res);
	MUGGLE_UNUSED(nbytes);
	return false;
}

static void
muggle_memory_res_destroy_huge_private(muggle_memory_resource_t *res)
{
	MUGGLE_UNUSED(res);
}

static bool muggle_memory_res_init_huge_share(muggle_memory_resource_t *res,
											  size_t nbytes, const char *k_name,
											  int k_num)
{
	MUGGLE_UNUSED(res);
	MUGGLE_UNUSED(nbytes);
	return false;
}

static void muggle_memory_res_destroy_huge_share(muggle_memory_resource_t *res)
{
	MUGGLE_UNUSED(res);
}

#endif

bool muggle_memory_res_init(muggle_memory_resource_t *res, size_t nbytes,
							uint64_t flags_val, const char *k_name, int k_num)
{
	memset(res, 0, sizeof(muggle_memory_resource_t));
	res->flags.val = flags_val;

	bool ret = true;
	switch (res->flags.mem_type) {
	case MUGGLE_MEMORY_RES_TYPE_HUGE_PRIVATE: {
		ret = muggle_memory_res_init_huge_private(res, nbytes);
	} break;
	case MUGGLE_MEMORY_RES_TYPE_HUGE_SHARE: {
		ret = muggle_memory_res_init_huge_share(res, nbytes, k_name, k_num);
	} break;
	default: {
		ret = muggle_memory_res_init_default(res, nbytes);
	} break;
	}

	if (ret) {
		// populate
#if MUGGLE_C_HAVE_MADV_POPULATE_WRITE
		if (res->flags.populate_type & MUGGLE_MEMORY_RES_POPULATE_WRITE) {
			madvise(res->data, res->n_bytes, MADV_POPULATE_WRITE);
		}
#endif

		// cal fields
		res->p_alloc = res->data;
		res->n_cache_line = res->n_bytes / MUGGLE_CACHE_LINE_SIZE;
		res->n_cache_line_remain = res->n_cache_line;
	}

	return ret;
}

void muggle_memory_res_destroy(muggle_memory_resource_t *res)
{
	if (res->data == NULL) {
		return;
	}

	switch (res->flags.mem_type) {
	case MUGGLE_MEMORY_RES_TYPE_HUGE_PRIVATE: {
		muggle_memory_res_destroy_huge_private(res);
	} break;
	case MUGGLE_MEMORY_RES_TYPE_HUGE_SHARE: {
		muggle_memory_res_destroy_huge_share(res);
	} break;
	default: {
		muggle_memory_res_destroy_default(res);
	} break;
	}
}

bool muggle_memory_res_rm_shm(const char *k_name, int k_num)
{
#if MUGGLE_PLATFORM_LINUX
	key_t shm_key = ftok(k_name, k_num);
	if (shm_key == -1) {
		return false;
	}

	int flag_open = 0;
	int flag_privilege = SHM_R | SHM_W;
	int flag_huge = SHM_HUGETLB;

	int shm_id = shmget(shm_key, 0, flag_open | flag_privilege | flag_huge);
	if (shm_id == -1) {
		return false;
	}

	if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
		return -1;
	}
#else
	MUGGLE_UNUSED(k_name);
	MUGGLE_UNUSED(k_num);
#endif
	return 0;
}

void *muggle_memory_res_alloc_cache_line(muggle_memory_resource_t *res,
										 size_t n)
{
	if (res->n_cache_line_remain < n) {
		return NULL;
	}

	void *p = res->p_alloc;
	res->p_alloc = (char *)res->p_alloc + MUGGLE_CACHE_LINE_SIZE * n;
	res->n_cache_line_remain -= n;
	return p;
}

void *muggle_memory_res_alloc(muggle_memory_resource_t *res, size_t nbytes)
{
	size_t n = MUGGLE_ALIGN_TRUE_SHARING(nbytes);
	return muggle_memory_res_alloc_cache_line(res, n);
}
