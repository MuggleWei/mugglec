#include "time_counter.h"
#include <string.h>

#if MUGGLE_PLATFORM_WINDOWS

bool muggle_time_counter_init(muggle_time_counter_t *tc)
{
	memset(tc, 0, sizeof(*tc));
	return QueryPerformanceFrequency(&tc->frequency) == TRUE ? true : false;
}

void muggle_time_counter_start(muggle_time_counter_t *tc)
{
	QueryPerformanceCounter(&tc->start);
}

void muggle_time_counter_end(muggle_time_counter_t *tc)
{
	QueryPerformanceCounter(&tc->end);
}

int64_t muggle_time_counter_interval_ns(muggle_time_counter_t *tc)
{
	double elapsed = (double)(tc->end.QuadPart - tc->start.QuadPart) /
					 (double)tc->frequency.QuadPart;
	return (int64_t)(elapsed * 1000000000);
}

int64_t muggle_time_counter_interval_ms(muggle_time_counter_t *tc)
{
	double elapsed = (double)(tc->end.QuadPart - tc->start.QuadPart) /
					 (double)tc->frequency.QuadPart;
	return (int64_t)(elapsed * 1000);
}

#else

bool muggle_time_counter_init(muggle_time_counter_t *tc)
{
	memset(tc, 0, sizeof(*tc));
	return true;
}

void muggle_time_counter_start(muggle_time_counter_t *tc)
{
#if MUGGLE_PLATFORM_APPLE
	clock_gettime(CLOCK_MONOTONIC, &tc->start_ts);
#else
	clock_gettime(CLOCK_BOOTTIME, &tc->start_ts);
#endif
}

void muggle_time_counter_end(muggle_time_counter_t *tc)
{
#if MUGGLE_PLATFORM_APPLE
	clock_gettime(CLOCK_MONOTONIC, &tc->end_ts);
#else
	clock_gettime(CLOCK_BOOTTIME, &tc->end_ts);
#endif
}

int64_t muggle_time_counter_interval_ns(muggle_time_counter_t *tc)
{
	int64_t elapsed = (tc->end_ts.tv_sec - tc->start_ts.tv_sec) * 1000000000LL +
					  tc->end_ts.tv_nsec - tc->start_ts.tv_nsec;
	return elapsed;
}

int64_t muggle_time_counter_interval_ms(muggle_time_counter_t *tc)
{
	int64_t elapsed = (tc->end_ts.tv_sec - tc->start_ts.tv_sec) * 1000LL +
					  ((tc->end_ts.tv_nsec - tc->start_ts.tv_nsec) / 1000000LL);
	return elapsed;
}

#endif
