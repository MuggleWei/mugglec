#include "win_gmtime.h"

#if MUGGLE_PLATFORM_WINDOWS

struct tm* gmtime_r(const time_t *timep, struct tm *result)
{
	errno_t err = gmtime_s(result, timep);
	if (err == 0)
	{
		return result;
	}

	return nullptr;
}

time_t timegm(struct tm *p_tm)
{
	return _mkgmtime(p_tm);
}

#endif
