/******************************************************************************
 *  @file         cpu.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-12-22
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec cpu handle
 *****************************************************************************/

#ifndef MUGGLE_C_OS_CPU_H_
#define MUGGLE_C_OS_CPU_H_

#include "muggle/c/base/macro.h"
#include <stdbool.h>
#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#elif MUGGLE_PLATFORM_APPLE
	#include <pthread.h>
	#include <mach/thread_policy.h>
#elif MUGGLE_PLATFORM_ANDROID
	#include <sched.h>
	#include <sys/types.h>
#else // *nix
	#include <sched.h>
#endif

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS
typedef HANDLE muggle_pid_handle_t;
typedef DWORD_PTR muggle_cpu_mask_t;
#elif MUGGLE_PLATFORM_APPLE
typedef mach_port_t muggle_pid_handle_t;
typedef thread_affinity_policy_data_t muggle_cpu_mask_t;
#elif MUGGLE_PLATFORM_ANDROID
typedef pid_t muggle_pid_handle_t;
typedef cpu_set_t muggle_cpu_mask_t;
#else // *nix
typedef pid_t muggle_pid_handle_t;
typedef cpu_set_t muggle_cpu_mask_t;
#endif

/**
 * @brief cleanup all CPU mask
 *
 * @param mask  cpu mask
 */
MUGGLE_C_EXPORT
void muggle_cpu_mask_zero(muggle_cpu_mask_t *mask);

/**
 * @brief add cpu to mask
 *
 * @param mask  cpu mask
 * @param cpu   cpu num
 */
MUGGLE_C_EXPORT
void muggle_cpu_mask_set(muggle_cpu_mask_t *mask, int cpu);

/**
 * @brief remove cpu from mask
 *
 * @param mask  cpu mask
 * @param cpu   cpu num
 */
MUGGLE_C_EXPORT
void muggle_cpu_mask_clr(muggle_cpu_mask_t *mask, int cpu);

/**
 * @brief check cpu is mask
 *
 * @param mask  cpu mask
 * @param cpu   cpu num
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_cpu_mask_isset(muggle_cpu_mask_t *mask, int cpu);

/**
 * @brief get current tid handle
 *
 * @return thread id handle
 */
MUGGLE_C_EXPORT
muggle_pid_handle_t muggle_get_current_tid_handle();

/**
 * @brief set a thread's CPU affinity mask
 *
 * @param tid   thread id, if it's zero, then the calling thread is used
 * @param mask  cpu mask
 *
 * @return
 *     - on success, return 0
 *     - otherwise -1 is returned and sys lasterror is set
 *
 * @NOTE
 *     - if in MACOS, mask support only one CPU
 */
MUGGLE_C_EXPORT
int muggle_cpu_set_thread_affinity(muggle_pid_handle_t tid,
								   const muggle_cpu_mask_t *mask);

/**
 * @brief get a thread's CPU affinity mask
 *
 * @param tid   thread id, if it's zero, then the calling thread is used
 * @param mask  cpu mask
 *
 * @return
 *     - on success, return 0
 *     - otherwise -1 is returned and sys lasterror is set
 */
MUGGLE_C_EXPORT
int muggle_cpu_get_thread_affinity(muggle_pid_handle_t tid,
								   muggle_cpu_mask_t *mask);

EXTERN_C_END

#endif // !MUGGLE_C_OS_CPU_H_
