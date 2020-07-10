/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_BENCHMARK_H_
#define MUGGLE_BENCHMARK_H_

#include "muggle/c/muggle_c.h"
#include <time.h>

 // lib and dll
#if MUGGLE_PLATFORM_WINDOWS && defined(MUGGLE_BENCHMARK_USE_DLL)
	#ifdef MUGGLE_BENCHMARK_EXPORTS
		#define MUGGLE_BENCHMARK_EXPORT __declspec(dllexport)
	#else
		#define MUGGLE_BENCHMARK_EXPORT __declspec(dllimport)
	#endif
#else
	#define MUGGLE_BENCHMARK_EXPORT
#endif

EXTERN_C_BEGIN

enum
{
	MUGGLE_BENCHMARK_ELAPSED_UNIT_NS,
	MUGGLE_BENCHMARK_ELAPSED_UNIT_CPU_CYCLE,
};

typedef struct muggle_benchmark_block
{
	uint64_t idx;
	union {
		struct timespec ts[8];
		uint64_t cpu_cycles[8];
	};
	uint64_t elapsed_ns;
} muggle_benchmark_block_t;

typedef struct muggle_benchmark_config
{
	char name[64];
	uint64_t loop;
	uint64_t cnt_per_loop;
	uint64_t loop_interval_ms;
	int report_step;
	int elapsed_unit;
} muggle_benchmark_config_t;

MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_gen_reports_head(FILE *fp, struct muggle_benchmark_config *config);

MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_gen_reports_body(
	FILE *fp,
	struct muggle_benchmark_config *config,
	struct muggle_benchmark_block *blocks,
	const char *case_name,
	uint64_t cnt,
	uint64_t ts_begin_idx,
	uint64_t ts_end_idx,
	bool sort
);

MUGGLE_BENCHMARK_EXPORT
uint64_t get_elapsed_ns(muggle_benchmark_block_t *block, int begin, int end);

MUGGLE_BENCHMARK_EXPORT
uint64_t get_elapsed_cpu_cycles(muggle_benchmark_block_t *block, int begin, int end);

EXTERN_C_END

#endif
