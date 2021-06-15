/******************************************************************************
 *  @file         atomic.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec atomic header file
 *
 *  Here contain typically atomic operator functions
 *****************************************************************************/

#ifndef MUGGLE_C_ATOMIC_H_
#define MUGGLE_C_ATOMIC_H_

#include "muggle/c/base/macro.h"

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

#define muggle_atomic_byte LONG
#define muggle_atomic_int LONG
#define muggle_atomic_int32 LONG
#define muggle_atomic_int64 LONG64

#define muggle_memory_order_relaxed 0
#define muggle_memory_order_consume 0
#define muggle_memory_order_acquire 0
#define muggle_memory_order_release 0
#define muggle_memory_order_acq_rel 0
#define muggle_memory_order_seq_cst 0

// load
#define muggle_atomic_load(ptr, memmodel) InterlockedOr(ptr, 0)

// store
#define muggle_atomic_store(ptr, val, memmodel) InterlockedExchange(ptr, val)

// exchange
#define muggle_atomic_exchange(ptr, val, memmodel) InterlockedExchange(ptr, val)
#define muggle_atomic_exchange32(ptr, val, memmodel) InterlockedExchange(ptr, val)
#define muggle_atomic_exchange64(ptr, val, memmodel) InterlockedExchange64(ptr, val)

// compare exchange
#define muggle_atomic_cmp_exch_weak(ptr, expected, desired, memmodel) muggle_win_atomic_cmp_exch32(ptr, expected, desired)
#define muggle_atomic_cmp_exch_weak32(ptr, expected, desired, memmodel) muggle_win_atomic_cmp_exch32(ptr, expected, desired)
#define muggle_atomic_cmp_exch_weak64(ptr, expected, desired, memmodel) muggle_win_atomic_cmp_exch64(ptr, expected, desired)

#define muggle_atomic_cmp_exch_strong(ptr, expected, desired, memmodel) muggle_win_atomic_cmp_exch32(ptr, expected, desired)
#define muggle_atomic_cmp_exch_strong32(ptr, expected, desired, memmodel) muggle_win_atomic_cmp_exch32(ptr, expected, desired)
#define muggle_atomic_cmp_exch_strong64(ptr, expected, desired, memmodel) muggle_win_atomic_cmp_exch64(ptr, expected, desired)

// fetch add
#define muggle_atomic_fetch_add(ptr, val, memmodel) InterlockedExchangeAdd(ptr, val)
#define muggle_atomic_fetch_add32(ptr, val, memmodel) InterlockedExchangeAdd(ptr, val)
#define muggle_atomic_fetch_add64(ptr, val, memmodel) InterlockedExchangeAdd64(ptr, val)

// fetch sub
#define muggle_atomic_fetch_sub(ptr, val, memmodel) InterlockedExchangeAdd(ptr, -val)
#define muggle_atomic_fetch_sub32(ptr, val, memmodel) InterlockedExchangeAdd(ptr, -val)
#define muggle_atomic_fetch_sub64(ptr, val, memmodel) InterlockedExchangeAdd64(ptr, -val)

// test_and_set
#define muggle_atomic_test_and_set(ptr, memmodel) !_interlockedbittestandset(ptr, 0)

// clear
#define muggle_atomic_clear(ptr, memmodel) _interlockedbittestandreset(ptr, 0)

// fence
#define muggle_atomic_thread_fence(memmodel) MemoryBarrier()
#define muggle_atomic_signal_fence(memmodel) MemoryBarrier()

EXTERN_C_BEGIN

MUGGLE_C_EXPORT
int muggle_win_atomic_cmp_exch32(muggle_atomic_int32 *dst, muggle_atomic_int32 *expected, muggle_atomic_int32 desired);

MUGGLE_C_EXPORT
int muggle_win_atomic_cmp_exch64(muggle_atomic_int64 *dst, muggle_atomic_int64 *expected, muggle_atomic_int64 desired);

EXTERN_C_END

#else

#define muggle_atomic_byte char // only use in test_and_set
#define muggle_atomic_int int
#define muggle_atomic_int32 int32_t
#define muggle_atomic_int64 int64_t

#define muggle_memory_order_relaxed __ATOMIC_RELAXED
#define muggle_memory_order_consume __ATOMIC_CONSUME
#define muggle_memory_order_acquire __ATOMIC_ACQUIRE
#define muggle_memory_order_release __ATOMIC_RELEASE
#define muggle_memory_order_acq_rel __ATOMIC_ACQ_REL
#define muggle_memory_order_seq_cst __ATOMIC_SEQ_CST

// load
#define muggle_atomic_load(ptr, memmodel) __atomic_load_n(ptr, memmodel)

// store
#define muggle_atomic_store(ptr, val, memmodel) __atomic_store_n(ptr, val, memmodel)

// exchange
#define muggle_atomic_exchange(ptr, val, memmodel) __atomic_exchange_n(ptr, val, memmodel)
#define muggle_atomic_exchange32(ptr, val, memmodel) __atomic_exchange_n(ptr, val, memmodel)
#define muggle_atomic_exchange64(ptr, val, memmodel) __atomic_exchange_n(ptr, val, memmodel)

// compare exchange
#define muggle_atomic_cmp_exch_weak(ptr, expected, desired, memmodel) __atomic_compare_exchange_n(ptr, expected, desired, 1, memmodel, __ATOMIC_RELAXED)
#define muggle_atomic_cmp_exch_weak32(ptr, expected, desired, memmodel) __atomic_compare_exchange_n(ptr, expected, desired, 1, memmodel, __ATOMIC_RELAXED)
#define muggle_atomic_cmp_exch_weak64(ptr, expected, desired, memmodel) __atomic_compare_exchange_n(ptr, expected, desired, 1, memmodel, __ATOMIC_RELAXED)

#define muggle_atomic_cmp_exch_strong(ptr, expected, desired, memmodel) __atomic_compare_exchange_n(ptr, expected, desired, 0, memmodel, __ATOMIC_RELAXED)
#define muggle_atomic_cmp_exch_strong32(ptr, expected, desired, memmodel) __atomic_compare_exchange_n(ptr, expected, desired, 0, memmodel, __ATOMIC_RELAXED)
#define muggle_atomic_cmp_exch_strong64(ptr, expected, desired, memmodel) __atomic_compare_exchange_n(ptr, expected, desired, 0, memmodel, __ATOMIC_RELAXED)

// fetch add
#define muggle_atomic_fetch_add(ptr, val, memmodel) __atomic_fetch_add (ptr, val, memmodel)
#define muggle_atomic_fetch_add32(ptr, val, memmodel) __atomic_fetch_add (ptr, val, memmodel)
#define muggle_atomic_fetch_add64(ptr, val, memmodel) __atomic_fetch_add (ptr, val, memmodel)

// fetch sub
#define muggle_atomic_fetch_sub(ptr, val, memmodel) __atomic_fetch_sub(ptr, val, memmodel)
#define muggle_atomic_fetch_sub32(ptr, val, memmodel) __atomic_fetch_sub(ptr, val, memmodel)
#define muggle_atomic_fetch_sub64(ptr, val, memmodel) __atomic_fetch_sub(ptr, val, memmodel)

// test_and_set
#define muggle_atomic_test_and_set(ptr, memmodel) !__atomic_test_and_set(ptr, memmodel)

// clear
#define muggle_atomic_clear(ptr, memmodel) __atomic_clear(ptr, memmodel)

// fence
#define muggle_atomic_thread_fence(memmodel) __atomic_thread_fence(memmodel)
#define muggle_atomic_signal_fence(memmodel) __atomic_signal_fence(memmodel)

#endif

#endif
