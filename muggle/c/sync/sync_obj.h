/******************************************************************************
 *  @file         sync_obj.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-11-24
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sync object
 *
 *  NOTE: Don't use it immediatelly, cause protability not guaranteed
 *****************************************************************************/

#ifndef MUGGLE_SYNC_OBJ_H_
#define MUGGLE_SYNC_OBJ_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>
#include <time.h>

EXTERN_C_BEGIN

#if MUGGLE_C_HAVE_LINUX_FUTEX || MUGGLE_C_HAVE_SYS_FUTEX

	#define MUGGLE_C_HAVE_SYNC_OBJ 1
	
	// NOTE
	// * sync_t use unsigned type
	//     Cause sync_t usually used as cursor in mugglec, use unsigned type
	//     can avoid undefined behavior for interger overflow
	// * futex(2): futex must be 32-bit
	//     A futex is a 32-bit value—referred to below as a futex word - whose 
	//     address is supplied to the futex() system call. (Futexes are 32-bits
	//     in size on all platforms, including 64-bit systems)
	// * futex(2): futex placed in region
	//     In multi-process program, the futex is placed in a region of shared 
	//     memory,  created using (for example) mmap(2) or shmat(2).
	//     In multi-threaded program, it is sufficient to place the futex word
	//     in a global variable shared by all threads.
	// * futex: memory barrier
	//     memory barrier and read are part of futex(FUTEX_WAKE), so no need to
	//     use atomic rel-acq for futex
	typedef uint32_t muggle_sync_t;

#elif MUGGLE_PLATFORM_WINDOWS

	#define MUGGLE_C_HAVE_SYNC_OBJ 1
	
	// NOTE
	// * The Risk of Signed integer overflow
	//     Cause Interlocked API use LONG, so if in Windows platform, need take
	//     into consider the risk of integer overflow
	// * Interlocked API: use LONG for Interlocked API
	//     Simple read and write to properly-aligned 32-bit variables are
	//     atomic operator. Reads and writes to 64-bit values are not 
	//     guaranteed to be atomic on 32-bit Windows.
	// * win synchapi
	//     The following synchronization functions use the appropriate barriers 
	//     to ensure memory ordering
	//     * Functions that enter or leave critical sections
	//     * Functions that acquire or release SRW locks
	//     * One-time initialization begin and completion
	//     * EnterSynchronizationBarrier function
	//     * Functions that signal synchronization objects
	//     * Wait functions
	//     * Interlocked functions (except functions with NoFence suffix, or 
	//       intrinsics with _nf suffix)
	typedef LONG muggle_sync_t

#else
	#define MUGGLE_C_HAVE_SYNC_OBJ 0
	typedef uint32_t muggle_sync_t
#endif

#if MUGGLE_C_HAVE_SYNC_OBJ

/**
 * @brief Waits for *addr != val
 *
 * @param addr     address on whitch to wait
 * @param val      compare value
 * @param timeout
 *     wait before the operation times out
 *     if NULL, the thread waits indefinitely until *addr changed
 *
 * @return 
 *     -1: failed
 *     otherwise: ok
 */
MUGGLE_C_EXPORT
int muggle_sync_wait(
	muggle_sync_t *addr, muggle_sync_t val, const struct timespec *timeout);

/**
 * @brief wake one thread that is waiting for the muggle_sync_t address to change
 *
 * @param addr  address on whitch to wait
 *
 * @return 
 *     -1: failed
 *     otherwise: ok
 */
MUGGLE_C_EXPORT
int muggle_sync_wake_one(muggle_sync_t *addr);

/**
 * @brief wake all threads that is waiting for the muggle_sync_t address to change
 *
 * @param addr  address on whitch to wait
 *
 * @return 
 *     -1: failed
 *     otherwise: ok
 */
MUGGLE_C_EXPORT
int muggle_sync_wake_all(muggle_sync_t *addr);

#endif

EXTERN_C_END

#endif // !MUGGLE_SYNC_OBJ_H_
