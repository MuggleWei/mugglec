/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_ATOMIC_H_
#define MUGGLE_C_ATOMIC_H_

#include "muggle/base_c/macro.h"

#if USE_C11_ATOMIC
	// TODO: 
#else
	#if MUGGLE_PLATFORM_WINDOWS

		#include <WinBase.h>

		// meaning: target = val;
		#define ATOMIC_Set_16(target, val) InterlockedExchange16(&target, val)
		#define ATOMIC_Set_32(target, val) InterlockedExchange(&target, val)
		#define ATOMIC_Set_64(target, val) InterlockedExchange64(&target, val)
		#define ATOMIC_Set_Pointer(target, val) InterlockedExchangePointer(&target, val)

		// meaning: ++target;
		#define ATOMIC_Increment_16(target) InterlockedIncrement16(&target)
		#define ATOMIC_Increment_32(target) InterlockedIncrement(&target)
		#define ATOMIC_Increment_64(target) InterlockedIncrement64(&target)
		
		// meaning: --target;
		#define ATOMIC_Decrement_16(target) InterlockedDecrement16(&target)
		#define ATOMIC_Decrement_32(target) InterlockedDecrement(&target)
		#define ATOMIC_Decrement_64(target) InterlockedDecrement64(&target)

		// meaning: target += val;
		#define ATOMIC_Add_32(target, val) InterlockedExchangeAdd(&target, val)
		#define ATOMIC_Add_64(target, val) InterlockedExchangeAdd64(&target, val)

		// meaning: target -= val;
		#define ATOMIC_Sub_32(target, val) InterlockedExchangeSubtract(&target, val)
		#define ATOMIC_Sub_64(target, val) InterlockedExchangeSubtract64(&target, val)

		// meaning: if (target == compare) { target = val; }
		#define ATOMIC_CAS_16(target, compare, val) InterlockedCompareExchange16(&target, val, compare)
		#define ATOMIC_CAS_32(target, compare, val) InterlockedCompareExchange(&target, val, compare)
		#define ATOMIC_CAS_64(target, compare, val) InterlockedCompareExchange64(&target, val, compare)
		#define ATOMIC_CAS_Pointer(target, compare, val) InterlockedCompareExchangePointer(&target, val, compare)

	#else

		// meaning: target = val
		#define ATOMIC_Set_16(target, val) __sync_val_compare_and_swap(&target, target, val)
		#define ATOMIC_Set_32(target, val) __sync_val_compare_and_swap(&target, target, val)
		#define ATOMIC_Set_64(target, val) __sync_val_compare_and_swap(&target, target, val)
		#define ATOMIC_Set_Pointer(target, val) __sync_val_compare_and_swap(&target, target, val)

		// meaning: ++target
		#define ATOMIC_Increment_16(target) __sync_add_and_fetch(&target, 1)
		#define ATOMIC_Increment_32(target) __sync_add_and_fetch(&target, 1)
		#define ATOMIC_Increment_64(target) __sync_add_and_fetch(&target, 1)

		// meaning: --target;
		#define ATOMIC_Decrement_16(target) __sync_sub_and_fetch(&target, 1)
		#define ATOMIC_Decrement_32(target) __sync_sub_and_fetch(&target, 1)
		#define ATOMIC_Decrement_64(target) __sync_sub_and_fetch(&target, 1)

		// meaning: target += val;
		#define ATOMIC_Add_32(target, val) __sync_add_and_fetch(&target, val)
		#define ATOMIC_Add_64(target, val) __sync_add_and_fetch(&target, val)

		// meaning: target -= val;
		#define ATOMIC_Sub_32(target, val) __sync_sub_and_fetch(&target, val)
		#define ATOMIC_Sub_64(target, val) __sync_sub_and_fetch(&target, val)

		// meaning: if (target == compare) { target = val; }
		#define ATOMIC_CAS_16(target, compare, val) __sync_val_compare_and_swap(&target, compare, val)
		#define ATOMIC_CAS_32(target, compare, val) __sync_val_compare_and_swap(&target, compare, val)
		#define ATOMIC_CAS_64(target, compare, val) __sync_val_compare_and_swap(&target, compare, val)
		#define ATOMIC_CAS_Pointer(target, compare, val) __sync_val_compare_and_swap(&target, compare, val)

	#endif
#endif

#endif