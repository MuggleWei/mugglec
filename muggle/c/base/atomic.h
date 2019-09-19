/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

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
#define muggle_atomic_load(ptr, memmodel) *(ptr)

// store
#define muggle_atomic_store(ptr, val, memmodel) *(ptr) = val

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

MUGGLE_CC_EXPORT
int muggle_win_atomic_cmp_exch32(muggle_atomic_int32 *dst, muggle_atomic_int32 *expected, muggle_atomic_int32 desired);

MUGGLE_CC_EXPORT
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

/////////////////////////////////////////////
// deprecated
#if USE_C11_ATOMIC
	// TODO: 
#else
	#if MUGGLE_PLATFORM_WINDOWS

		#include <WinBase.h>

		// meaning: target = val;
		#define MUGGLE_ATOMIC_Set(target, val) InterlockedExchange(&target, val)
		#define MUGGLE_ATOMIC_Set_16(target, val) InterlockedExchange16(&target, val)
		#define MUGGLE_ATOMIC_Set_32(target, val) InterlockedExchange(&target, val)
		#define MUGGLE_ATOMIC_Set_64(target, val) InterlockedExchange64(&target, val)
		#define MUGGLE_ATOMIC_Set_Pointer(target, val) InterlockedExchangePointer(&target, val)

		// meaning: ++target;
		#define MUGGLE_ATOMIC_Increment_And_Fetch(target) InterlockedIncrement(&target)
		#define MUGGLE_ATOMIC_Increment_And_Fetch_16(target) InterlockedIncrement16(&target)
		#define MUGGLE_ATOMIC_Increment_And_Fetch_32(target) InterlockedIncrement(&target)
		#define MUGGLE_ATOMIC_Increment_And_Fetch_64(target) InterlockedIncrement64(&target)
		
		// meaning: --target;
		#define MUGGLE_ATOMIC_Decrement_And_Fetch(target) InterlockedDecrement(&target)
		#define MUGGLE_ATOMIC_Decrement_And_Fetch_16(target) InterlockedDecrement16(&target)
		#define MUGGLE_ATOMIC_Decrement_And_Fetch_32(target) InterlockedDecrement(&target)
		#define MUGGLE_ATOMIC_Decrement_And_Fetch_64(target) InterlockedDecrement64(&target)

		// meaning: target += val;
		#define MUGGLE_ATOMIC_Fetch_And_Add(target, val) InterlockedExchangeAdd(&target, val)
		#define MUGGLE_ATOMIC_Fetch_And_Add_32(target, val) InterlockedExchangeAdd(&target, val)
		#define MUGGLE_ATOMIC_Fetch_And_Add_64(target, val) InterlockedExchangeAdd64(&target, val)

		// meaning: target -= val;
		#define MUGGLE_ATOMIC_Fetch_And_Sub(target, val) InterlockedExchangeSubtract(&target, val)
		#define MUGGLE_ATOMIC_Fetch_And_Sub_32(target, val) InterlockedExchangeSubtract(&target, val)
		#define MUGGLE_ATOMIC_Fetch_And_Sub_64(target, val) InterlockedExchangeSubtract64(&target, val)

		// meaning: if (target == compare) { target = val; }
		#define MUGGLE_ATOMIC_CAS(target, compare, val) InterlockedCompareExchange(&target, val, compare)
		#define MUGGLE_ATOMIC_CAS_16(target, compare, val) InterlockedCompareExchange16(&target, val, compare)
		#define MUGGLE_ATOMIC_CAS_32(target, compare, val) InterlockedCompareExchange(&target, val, compare)
		#define MUGGLE_ATOMIC_CAS_64(target, compare, val) InterlockedCompareExchange64(&target, val, compare)
		#define MUGGLE_ATOMIC_CAS_Pointer(target, compare, val) InterlockedCompareExchangePointer(&target, val, compare)

	#else

		// meaning: target = val
		#define MUGGLE_ATOMIC_Set(target, val) __sync_val_compare_and_swap(&target, target, val)
		#define MUGGLE_ATOMIC_Set_16(target, val) __sync_val_compare_and_swap(&target, target, val)
		#define MUGGLE_ATOMIC_Set_32(target, val) __sync_val_compare_and_swap(&target, target, val)
		#define MUGGLE_ATOMIC_Set_64(target, val) __sync_val_compare_and_swap(&target, target, val)
		#define MUGGLE_ATOMIC_Set_Pointer(target, val) __sync_val_compare_and_swap(&target, target, val)

		// meaning: ++target
		#define MUGGLE_ATOMIC_Increment_And_Fetch(target) __sync_add_and_fetch(&target, 1)
		#define MUGGLE_ATOMIC_Increment_And_Fetch_16(target) __sync_add_and_fetch(&target, 1)
		#define MUGGLE_ATOMIC_Increment_And_Fetch_32(target) __sync_add_and_fetch(&target, 1)
		#define MUGGLE_ATOMIC_Increment_And_Fetch_64(target) __sync_add_and_fetch(&target, 1)

		// meaning: --target;
		#define MUGGLE_ATOMIC_Decrement_And_Fetch(target) __sync_sub_and_fetch(&target, 1)
		#define MUGGLE_ATOMIC_Decrement_And_Fetch_16(target) __sync_sub_and_fetch(&target, 1)
		#define MUGGLE_ATOMIC_Decrement_And_Fetch_32(target) __sync_sub_and_fetch(&target, 1)
		#define MUGGLE_ATOMIC_Decrement_And_Fetch_64(target) __sync_sub_and_fetch(&target, 1)

		// meaning: target += val;
		#define MUGGLE_ATOMIC_Fetch_And_Add(target, val) __sync_fetch_and_add(&target, val)
		#define MUGGLE_ATOMIC_Fetch_And_Add_32(target, val) __sync_fetch_and_add(&target, val)
		#define MUGGLE_ATOMIC_Fetch_And_Add_64(target, val) __sync_fetch_and_add(&target, val)

		// meaning: target -= val;
		#define MUGGLE_ATOMIC_Fetch_And_Sub(target, val) __sync_fetch_and_sub(&target, val)
		#define MUGGLE_ATOMIC_Fetch_And_Sub_32(target, val) __sync_fetch_and_sub(&target, val)
		#define MUGGLE_ATOMIC_Fetch_And_Sub_64(target, val) __sync_fetch_and_sub(&target, val)

		// meaning: if (target == compare) { target = val; }
		#define MUGGLE_ATOMIC_CAS(target, compare, val) __sync_val_compare_and_swap(&target, compare, val)
		#define MUGGLE_ATOMIC_CAS_16(target, compare, val) __sync_val_compare_and_swap(&target, compare, val)
		#define MUGGLE_ATOMIC_CAS_32(target, compare, val) __sync_val_compare_and_swap(&target, compare, val)
		#define MUGGLE_ATOMIC_CAS_64(target, compare, val) __sync_val_compare_and_swap(&target, compare, val)
		#define MUGGLE_ATOMIC_CAS_Pointer(target, compare, val) __sync_val_compare_and_swap(&target, compare, val)

	#endif
#endif

#endif
