#include "cpu.h"

#if MUGGLE_PLATFORM_WINDOWS

void muggle_cpu_mask_zero(muggle_cpu_mask_t *mask)
{
	memset(mask, 0, sizeof(*mask));
}

void muggle_cpu_mask_set(muggle_cpu_mask_t *mask, int cpu)
{
	(*mask) |= (0x01 << cpu);
}

void muggle_cpu_mask_clr(muggle_cpu_mask_t *mask, int cpu)
{
	(*mask) &= (~(0x01 << cpu));
}

bool muggle_cpu_mask_isset(muggle_cpu_mask_t *mask, int cpu)
{
	return (*mask) & (0x01 << cpu);
}

muggle_pid_handle_t muggle_get_current_tid_handle()
{
	return GetCurrentThread();
}

int muggle_cpu_set_thread_affinity(muggle_pid_handle_t tid,
								   const muggle_cpu_mask_t *mask)
{
	if ((void *)tid == NULL) {
		tid = muggle_get_current_tid_handle();
	}
	return SetThreadAffinityMask(tid, *mask) != 0 ? 0 : -1;
}

int muggle_cpu_get_thread_affinity(muggle_pid_handle_t tid,
								   muggle_cpu_mask_t *mask)
{
	muggle_cpu_mask_t old = 0, tmp = 1;

	if ((void *)tid == NULL) {
		tid = muggle_get_current_tid_handle();
	}

	while (tmp) {
		old = SetThreadAffinityMask(tid, tmp);
		if (old) {
			SetThreadAffinityMask(tid, old); //!< restore
			*mask = old;
			return 0;
		} else {
			if (GetLastError() != ERROR_INVALID_PARAMETER) {
				return - 1;
			}
		}

		tmp <<= 1;
		if (tmp > (0x01 << 30)) {
			break;
		}
	}

	return -1;
}

#elif MUGGLE_PLATFORM_APPLE

#include <mach/mach.h>

void muggle_cpu_mask_zero(muggle_cpu_mask_t *mask)
{
    mask->affinity_tag = 0;
}

void muggle_cpu_mask_set(muggle_cpu_mask_t *mask, int cpu)
{
    mask->affinity_tag = cpu;
}

void muggle_cpu_mask_clr(muggle_cpu_mask_t *mask, int cpu)
{
    if (muggle_cpu_mask_isset(mask, cpu)) {
        mask->affinity_tag = 0;
    }
}

bool muggle_cpu_mask_isset(muggle_cpu_mask_t *mask, int cpu)
{
    return mask->affinity_tag == cpu;
}

muggle_pid_handle_t muggle_get_current_tid_handle()
{
    return mach_thread_self();
}

int muggle_cpu_set_thread_affinity(muggle_pid_handle_t tid,
								   const muggle_cpu_mask_t *mask)
{
	if ((void *)tid == NULL) {
		tid = muggle_get_current_tid_handle();
	}
    kern_return_t ret = thread_policy_set(tid, THREAD_AFFINITY_POLICY,
            (thread_policy_t)mask, THREAD_AFFINITY_POLICY_COUNT);
    return ret == KERN_SUCCESS ? 0 : -1;
}

int muggle_cpu_get_thread_affinity(muggle_pid_handle_t tid,
								   muggle_cpu_mask_t *mask)
{
	if ((void *)tid == NULL) {
		tid = muggle_get_current_tid_handle();
	}
    mach_msg_type_number_t count = 0;
    boolean_t get_default = false;
    kern_return_t ret = thread_policy_get(tid, THREAD_AFFINITY_POLICY,
            (thread_policy_t)mask, &count, &get_default);
    return ret == KERN_SUCCESS ? 0 : -1;
}

#else // android or *nix

#include <unistd.h>

void muggle_cpu_mask_zero(muggle_cpu_mask_t *mask)
{
	CPU_ZERO(mask);
}

void muggle_cpu_mask_set(muggle_cpu_mask_t *mask, int cpu)
{
	CPU_SET(cpu, mask);
}

void muggle_cpu_mask_clr(muggle_cpu_mask_t *mask, int cpu)
{
	CPU_CLR(cpu, mask);
}

bool muggle_cpu_mask_isset(muggle_cpu_mask_t *mask, int cpu)
{
	return CPU_ISSET(cpu, mask) != 0;
}

muggle_pid_handle_t muggle_get_current_tid_handle()
{
	return gettid();
}

int muggle_cpu_set_thread_affinity(muggle_pid_handle_t tid,
								   const muggle_cpu_mask_t *mask)
{
	return sched_setaffinity(tid, CPU_SETSIZE, mask);
}

int muggle_cpu_get_thread_affinity(muggle_pid_handle_t tid,
								   muggle_cpu_mask_t *mask)
{
	return sched_getaffinity(tid, CPU_SETSIZE, mask);
}

#endif
