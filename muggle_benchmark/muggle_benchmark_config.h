/******************************************************************************
 *  @file         muggle_benchmark_config.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-01-02
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec benchmark config
 *****************************************************************************/

#ifndef MUGGLE_C_BENCHMARK_CONFIG_H_
#define MUGGLE_C_BENCHMARK_CONFIG_H_

#include "muggle_benchmark/muggle_benchmark_macro.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_BENCHMARK_ELAPSED_UNIT_NS, //!< elapsed unit with nano second
	MUGGLE_BENCHMARK_ELAPSED_UNIT_CPU_CYCLE, //!, elapsed unit with cpu cycle
};

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
 * @brief parse command line
 *
 * @param config  mugglec benchmark config
 * @param argc    command line argc
 * @param argv    command line argv
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_config_parse_cli(
	muggle_benchmark_config_t *config,
	int argc, char *argv[]);

/**
 * @brief output benchmark config
 *
 * @param config  mugglec benchmark config
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_config_output(muggle_benchmark_config_t *config);

EXTERN_C_BEGIN

#endif // !MUGGLE_C_BENCHMARK_CONFIG_H_
