#include "muggle/base/sleep.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif

void SleepFunction(unsigned long ms)
{
#if MUGGLE_PLATFORM_WINDOWS
	Sleep(ms);
#else
	usleep((double)(ms) * 1000.0);
#endif
}