#include "sync_obj.h"

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>
#include <time.h>

int muggle_sync_wait(
	muggle_sync_t *addr, muggle_sync_t val, const struct timespec *timeout)
{
	BOOL ret;
	if (timeout == NULL)
	{
		ret = WaitOnAddress(addr, &val, sizeof(muggle_sync_t), INFINITE);
	}
	else
	{
		DWORD ms = timeout->tv_nsec / 1000000;
		if (ms == 0 && timeout->tv_nsec != 0)
		{
			ms = 1;
		}
		DWORD dwMilliseconds = (DWORD)(timeout->tv_sec * 1000 + ms);
		ret = WaitOnAddress(addr, &val, sizeof(val), dwMilliseconds);
	}

	return ret ? 0 : 1;
}

int muggle_sync_wake_one(muggle_sync_t *addr)
{
	WakeByAddressSingle(addr);
	return 0;
}

int muggle_sync_wake_all(muggle_sync_t *addr)
{
	WakeByAddressAll(addr);
	return 0;
}

#endif
