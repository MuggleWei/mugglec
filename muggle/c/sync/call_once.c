#include "call_once.h"

void muggle_call_once(muggle_once_flag *flag, muggle_once_func func)
{
	muggle_atomic_byte v = MUGGLE_ONCE_FLAG_INIT;
	if (muggle_atomic_cmp_exch_strong(flag, &v, MUGGLE_ONCE_FLAG_WAIT,
									  muggle_memory_order_relaxed)) {
		func();
		muggle_atomic_store(flag, MUGGLE_ONCE_FLAG_READY,
							muggle_memory_order_release);
	} else {
		do {
			v = muggle_atomic_load(flag, muggle_memory_order_acquire);
		} while (v != MUGGLE_ONCE_FLAG_READY);
	}
}
