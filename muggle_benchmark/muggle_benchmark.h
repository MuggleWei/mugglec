/******************************************************************************
 *  @file         muggle_benchmark.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-12-31
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec benchmark
 *****************************************************************************/

#ifndef MUGGLE_C_BENCHMARK_H_
#define MUGGLE_C_BENCHMARK_H_

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
	MUGGLE_BENCHMARK_ELAPSED_UNIT_NS, //!< elapsed unit with nano second
	MUGGLE_BENCHMARK_ELAPSED_UNIT_CPU_CYCLE, //!, elapsed unit with cpu cycle
};

/**
 * @brief benchmark record
 */
typedef struct muggle_benchmark_record
{
	uint64_t idx;    //!< record index
	uint64_t action; //!< record action
	union {
		struct timespec ts;  //!< elapsed time with timespec
		uint64_t cpu_cycles; //!< elapsed time with cpu cycle
	};
} muggle_benchmark_record_t;

/**
 * @brief benchmark config
 */
typedef struct muggle_benchmark_config
{
	uint64_t rounds;            //!< total rounds in benchmark
	uint64_t record_per_round;  //!< how many times run in a round
	int      round_interval_ms; //!< round interval in milliseconds
	int      elapsed_unit;      //!< elapsed time unit
	int      capacity;          //!< [optional] usually refers to buffer capacity
	int      producer;          //!< [optional] number of producer
	int      consumer;          //!< [optional] number of consumer
	int      report_step;       //!< sampling step in report
} muggle_benchmark_config_t;

/**
 * @brief generate benchmark records report
 *
 * @param fp            output file handle
 * @param action_name[] action name array
 * @param elapsed_unit  elapsed time unit
 * @param cnt_array     number of record array
 * @param cnt_record    number of records in single array
 * @param records[]     records arrays
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_gen_records_report(
	FILE *fp,
	const char *action_name[],
	int elapsed_unit,
	int cnt_array,
	int cnt_record,
	muggle_benchmark_record_t *records[]);

/**
 * @brief generate latency benchmark report head
 *
 * @param fp      output file handle
 * @param config  benchmark config
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_gen_latency_report_head(FILE *fp, struct muggle_benchmark_config *config);

/**
 * @brief generate latency benchmark report body
 *
 * @param fp            output file handle
 * @param action_name[] action name array
 * @param config        benchmark config
 * @param cnt_record    number of record in single array
 * @param rs1           record array 1
 * @param rs2           record array 2
 * @param sort_result   whether need to sort result
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_gen_latency_report_body(
	FILE *fp,
	const char *action_name[],
	struct muggle_benchmark_config *config,
	uint64_t cnt_record,
	muggle_benchmark_record_t *rs1,
	muggle_benchmark_record_t *rs2,
	bool sort_result);

/**
 * @brief get elapsed time in unit nanosecond
 *
 * @param r1  record1
 * @param r2  record2
 *
 * @return record2 - record1 in nanosecond
 */
MUGGLE_BENCHMARK_EXPORT
uint64_t muggle_benchmark_get_elapsed_ns(muggle_benchmark_record_t *r1, muggle_benchmark_record_t *r2);

/**
 * @brief get elapsed time in unit cpu cycle
 *
 * @param r1  record1
 * @param r2  record2
 *
 * @return record2 - record1 in cpu cycle
 */
MUGGLE_BENCHMARK_EXPORT
uint64_t muggle_benchmark_get_elapsed_cpu_cycles(muggle_benchmark_record_t *r1, muggle_benchmark_record_t *r2);

EXTERN_C_END

#endif
