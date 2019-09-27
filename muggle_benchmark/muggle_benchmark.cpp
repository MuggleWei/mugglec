/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle_benchmark.h"

NS_MUGGLE_BEGIN

void GenLatencyReportsHead(
	FILE *fp,
	BenchmarkConfig *config
)
{
	fwrite("case_name", 1, strlen("case_name"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("loop", 1, strlen("loop"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("cnt_per_loop", 1, strlen("cnt_per_loop"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("loop_interval_ms", 1, strlen("loop_interval_ms"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("avg", 1, strlen("avg"), fp);
	fwrite(",", 1, strlen(","), fp);
	for (int i = 0; i < 100; i += config->report_step)
	{
		char buf[16] = {0};
		snprintf(buf, sizeof(buf) - 1, "%d", i);
		fwrite(buf, 1, strlen(buf), fp);
		fwrite(",", 1, strlen(","), fp);
	}
	fwrite("100", 1, strlen("100"), fp);
	fwrite("\n", 1, strlen("\n"), fp);
}

static int compare_uint64(const void *a, const void *b)
{
	const uint64_t &arg1 = *(const uint64_t*)a;
	const uint64_t &arg2 = *(const uint64_t*)b;

	if (arg1 < arg2) return -1;
	if (arg1 > arg2) return 1;
	return 0;
}

void GenLatencyReportsBody(
	FILE *fp,
	BenchmarkConfig *config,
	LatencyBlock *blocks,
	const char *case_name,
	uint64_t cnt,
	uint64_t ts_begin_idx,
	uint64_t ts_end_idx,
	bool sort
)
{
	uint64_t *elapseds = (uint64_t*)malloc(cnt * sizeof(uint64_t));
	uint64_t sum = 0, cnt_sum = 0, avg = 0;
	for (uint64_t i = 0; i < cnt; ++i)
	{
		if (blocks[i].ts[ts_end_idx].tv_sec == 0 || blocks[i].ts[ts_begin_idx].tv_sec == 0)
		{
			elapseds[i] = UINT_MAX;
		}
		else
		{
			elapseds[i] = 
				(blocks[i].ts[ts_end_idx].tv_sec - blocks[i].ts[ts_begin_idx].tv_sec) * 1000000000 +
				blocks[i].ts[ts_end_idx].tv_nsec - blocks[i].ts[ts_begin_idx].tv_nsec;
			sum += elapseds[i];
			cnt_sum++;
		}
	}
	avg = sum / cnt_sum;

	if (sort)
	{
		qsort(elapseds, cnt, sizeof(uint64_t), compare_uint64);
	}

	char buf[4096] = {0};
	snprintf(buf, sizeof(buf) - 1, "%s,%llu,%llu,%llu,%llu,",
		case_name,
		(unsigned long long)config->loop,
		(unsigned long long)config->cnt_per_loop,
		(unsigned long long)config->loop_interval_ms,
		(unsigned long long)avg);
	fwrite(buf, 1, strlen(buf), fp);

	for (int i = 0; i < 100; i += config->report_step)
	{
		uint64_t idx = (uint64_t)((i / 100.0) * cnt);
		if (elapseds[idx] == UINT_MAX)
		{
			snprintf(buf, sizeof(buf) - 1, "-");
		}
		else
		{
			snprintf(buf, sizeof(buf) - 1, "%llu", (unsigned long long)elapseds[idx]);
		}
		fwrite(buf, 1, strlen(buf), fp);
		fwrite(",", 1, strlen(","), fp);
	}
	if (elapseds[cnt-1] == UINT_MAX)
	{
		snprintf(buf, sizeof(buf) - 1, "-");
	}
	else
	{
		snprintf(buf, sizeof(buf) - 1, "%llu", (unsigned long long)elapseds[cnt-1]);
	}
	fwrite(buf, 1, strlen(buf), fp);
	fwrite("\n", 1, strlen("\n"), fp);

	free(elapseds);
}

NS_MUGGLE_END
