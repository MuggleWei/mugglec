/******************************************************************************
 *  @file         muggle_benchmark_handle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-01-02
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec benchmark handle
 *****************************************************************************/

#ifndef MUGGLE_C_BENCHMARK_HANDLE_H_
#define MUGGLE_C_BENCHMARK_HANDLE_H_

#include "muggle_benchmark_macro.h"
#include "muggle_benchmark/muggle_benchmark_config.h"

EXTERN_C_BEGIN

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
 * @brief callback function for record
 *
 * @param record  benchmark record
 */
typedef void (*fn_muggle_benchmark_record)(muggle_benchmark_record_t *record);

/**
 * @brief benchmark handle
 */
typedef struct muggle_benchmark_handle
{
	uint64_t                   record_count;     //!< total number of record
	int                        action_count;     //!< total action count
	char                       **action_name;    //!< action name map
	muggle_benchmark_record_t  **action_records; //!< action to records array map
} muggle_benchmark_handle_t;

/**
 * @brief initialize benchmark handle
 *
 * @param handle        benchmark handle
 * @param record_count  total number of record
 * @param action_count  total action count
 *
 * @return 
 *   0 - success
 *   otherwise - failed
 */
MUGGLE_BENCHMARK_EXPORT
int muggle_benchmark_handle_init(
	muggle_benchmark_handle_t *handle,
	uint64_t record_count,
	int action_count);

/**
 * @brief destroy benchmark handle)
 *
 * @param handle  benchmark handle
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_handle_destroy(muggle_benchmark_handle_t *handle);

/**
 * @brief set action name and allocate records space
 *
 * @param handle  benchmark handle
 * @param action      action id
 * @param action_name action name
 *
 * @return 
 *   0 - seccess
 *   otherwise - failed
 */
MUGGLE_BENCHMARK_EXPORT
int muggle_benchmark_handle_set_action(muggle_benchmark_handle_t *handle, int action, const char *action_name);

/**
 * @brief get record count
 *
 * @param handle  benchmark handle
 * @return record count
 */
MUGGLE_BENCHMARK_EXPORT
uint64_t muggle_benchmark_handle_get_record_count(muggle_benchmark_handle_t *handle);

/**
 * @brief get action name
 *
 * @param handle  benchmark handle
 * @param action  action id
 *
 * @return action name
 */
MUGGLE_BENCHMARK_EXPORT
const char* muggle_benchmark_handle_get_action_name(muggle_benchmark_handle_t *handle, int action);

/**
 * @brief get records by action id
 *
 * @param handle  benchmark handle
 * @param action  action id
 *
 * @return records
 */
MUGGLE_BENCHMARK_EXPORT
muggle_benchmark_record_t* muggle_benchmark_handle_get_records(muggle_benchmark_handle_t *handle, int action);

/**
 * @brief generate records report
 *
 * @param handle       benchmark handle
 * @param fp           output file handle
 * @param elapsed_unit elapsed time unit
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_handle_gen_records_report(
	muggle_benchmark_handle_t *handle,
	FILE *fp,
	int elapsed_unit);

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
 * @param handle        benchmark handle
 * @param config        benchmark config
 * @param action1       action id1
 * @param action2       action id2
 * @param sort_result   whether need to sort result
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_gen_latency_report_body(
	FILE *fp,
	muggle_benchmark_handle_t *handle,
	struct muggle_benchmark_config *config,
	int action1,
	int action2,
	bool sort_result);

/**
 * @brief get record function
 *
 * @param elapsed_unit elapsed unit
 *
 * @return record function
 */
MUGGLE_BENCHMARK_EXPORT
fn_muggle_benchmark_record muggle_benchmark_get_fn_record(int elapsed_unit);

EXTERN_C_END

#endif // !MUGGLE_C_BENCHMARK_HANDLE_H_
