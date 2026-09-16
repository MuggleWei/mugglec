/******************************************************************************
 *  @file         memory_resource.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2026-09-16
 *  @copyright    Copyright 2026 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec memory resource
 *****************************************************************************/

#ifndef MUGGLE_C_MEMORY_RESOURCE_H_
#define MUGGLE_C_MEMORY_RESOURCE_H_

#include "muggle/c/base/macro.h"
#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

EXTERN_C_BEGIN

enum {
	MUGGLE_MEMORY_RES_TYPE_NULL = 0, //!< ordinary memory
	MUGGLE_MEMORY_RES_TYPE_HUGE_PRIVATE = 1, //!< private huge page
	MUGGLE_MEMORY_RES_TYPE_HUGE_SHARE = 2, //!< share huge page
};

enum {
	MUGGLE_MEMORY_RES_HUGE_DEFAULT = 0, //!< default huge page
	MUGGLE_MEMORY_RES_HUGE_2MB = 1, //!< 2MB huge page
	MUGGLE_MEMORY_RES_HUGE_1GB = 2, //!< 1GB huge page
};

enum {
	MUGGLE_MEMORY_RES_POPULATE_NULL = 0x00, //!< without any populate when init
	MUGGLE_MEMORY_RES_POPULATE_WRITE = 0x01, //!< populate write when init
};

enum {
	MUGGLE_MEMORY_RES_SHM_NULL = 0x00,
	MUGGLE_MEMORY_RES_SHM_CREATE = 0x01, //!< create share memory
	MUGGLE_MEMORY_RES_SHM_PRIVILEGE_GROUP = 0x02, //!< support group read/write
	MUGGLE_MEMORY_RES_SHM_PRIVILEGE_OTHER = 0x04, //!< support other read/write
};

#define MUGGLE_MEMORY_RES_PAGE_SIZE_2MB (2ull * 1024ull * 1024ull)
#define MUGGLE_MEMORY_RES_PAGE_SIZE_1GB (1ull * 1024ull * 1024ull * 1024ull)

typedef union {
	uint64_t val;
	struct {
		uint8_t mem_type; //!< memory type; MUGGLE_MEMORY_RES_TYPE_*
		uint8_t huge_type; //!< huge page type; MUGGLE_MEMORY_RES_HUGE_*
		uint8_t populate_type; //!< populate type
		uint8_t share_flag; //!< huge page share flag
	};
} muggle_memory_res_flags_t;

typedef struct {
#if MUGGLE_PLATFORM_WINDOWS
	HANDLE hMapFile;
#else
	int shm_id; //!< shared memory id
#endif
} muggle_memory_res_shm_info_t;

typedef struct {
	muggle_memory_res_shm_info_t shm; //!< share memory info
	muggle_memory_res_flags_t flags; //!< flags
	void *data; //!< memory data
	void *p_alloc; //!< alloc pointer
	size_t n_bytes; //!< number bytes of memory
	size_t n_cache_line; //!< number of cache line
	size_t n_cache_line_remain; //!< number of remain cache line
} muggle_memory_resource_t;

/**
 * @brief initialize memory resource
 *
 * @param res     memory resource
 * @param nbytes  number of bytes
 * @param flags   muggle_memory_res_flags_t::val
 * @param k_name  key's name of shared memory
 *                  - use with MUGGLE_MEMORY_RES_TYPE_HUGE_SHARE
 *                  - in *nix, gurantee it's an exists filepath
 * @param k_num   key's number
 *                  - use with MUGGLE_MEMORY_RES_TYPE_HUGE_SHARE
 *                  - range in [1, 255]
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_memory_res_init(muggle_memory_resource_t *res, size_t nbytes,
							uint64_t flags, const char *k_name, int k_num);

/**
 * @brief destroy memory resource
 *
 * @param res  memory resource
 */
MUGGLE_C_EXPORT
void muggle_memory_res_destroy(muggle_memory_resource_t *res);

/**
 * @brief remove share memory in system
 *
 * @param k_name  key's name of shared memory
 *                  - use with MUGGLE_MEMORY_RES_TYPE_HUGE_SHARE
 *                  - in *nix, gurantee it's an exists filepath
 * @param k_num   key's number
 *                  - use with MUGGLE_MEMORY_RES_TYPE_HUGE_SHARE
 *                  - range in [1, 255]
 */
MUGGLE_C_EXPORT
bool muggle_memory_res_rm_shm(const char *k_name, int k_num);

/**
 * @brief allocate n cache line
 *
 * @param res  memory resource
 * @param n    number of cache line
 *
 * @return
 *   - on success, return memory address
 *   - on failed, return NULL
 */
MUGGLE_C_EXPORT
void *muggle_memory_res_alloc_cache_line(muggle_memory_resource_t *res,
										 size_t n);

/**
 * @brief allocate bytes
 *
 * @param res     memory resource
 * @param nbytes  number of cache line
 *
 * @return
 *   - on success, return memory address
 *   - on failed, return NULL
 */
MUGGLE_C_EXPORT
void *muggle_memory_res_alloc(muggle_memory_resource_t *res, size_t nbytes);

EXTERN_C_END

#endif // !MUGGLE_C_MEMORY_RESOURCE_H_
