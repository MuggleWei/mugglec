/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_BENCHMARK_H_
#define MUGGLE_BENCHMARK_H_

#include "muggle/muggle_cc.h"
#include <time.h>

NS_MUGGLE_BEGIN

struct LatencyBlock
{
	uint64_t idx;
	struct timespec ts[8];
	uint64_t elapsed_ns;
};

struct BenchmarkConfig
{
	char name[32];
	uint64_t loop;
	uint64_t cnt_per_loop;
	uint64_t loop_interval_ms;
	int report_step;
};

void GenLatencyReportsHead(
	FILE *fp,
	BenchmarkConfig *config
);

void GenLatencyReportsBody(
	FILE *fp,
	BenchmarkConfig *config,
	LatencyBlock *blocks,
	const char *case_name,
	uint64_t cnt,
	uint64_t ts_begin_idx,
	uint64_t ts_end_idx,
	bool sort
);

NS_MUGGLE_END

#endif
