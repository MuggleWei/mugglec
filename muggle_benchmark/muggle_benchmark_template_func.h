/******************************************************************************
 *  @file         muggle_benchmark_template_func.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-01-05
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec benchmark function template
 *****************************************************************************/

#ifndef MUGGLE_C_BENCHMARK_TEMPLATE_FUNC_H_
#define MUGGLE_C_BENCHMARK_TEMPLATE_FUNC_H_

#include "muggle_benchmark/muggle_benchmark_macro.h"
#include "muggle_benchmark/muggle_benchmark_config.h"
#include "muggle_benchmark/muggle_benchmark_handle.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_BENCHMARK_FUNC_ACTION_BEFORE,
	MUGGLE_BENCHMARK_FUNC_ACTION_AFTER,
	MAX_MUGGLE_BENCHMARK_FUNC_ACTION,
};

/**
 * @brief function prototype of function
 *
 * @param args  user data
 * @param idx   index of operation
 *
 * @return 
 */
typedef void* (*fn_muggle_benchmark_func)(void *args, uint64_t idx);

typedef struct muggle_benchmark_func
{
	muggle_benchmark_config_t *config; //!< benchmark config
	muggle_benchmark_handle_t handle;  //!< benchmark handle
	void                      *args;   //!< user data
	fn_muggle_benchmark_func  func;    //!< function
} muggle_benchmark_func_t;

/**
 * @brief initialize function benchmark handle
 *
 * @param benchmark  function benchmark handle
 * @param config     benchmark config
 * @param args       user data
 * @param func       function
 *
 * @return 
 *     0 - success
 *     otherwise - failed
 */
MUGGLE_BENCHMARK_EXPORT
int muggle_benchmark_func_init(
	muggle_benchmark_func_t *benchmark,
	muggle_benchmark_config_t *config,
	void *args,
	fn_muggle_benchmark_func func);

/**
 * @brief destroy function benchmark handle
 *
 * @param benchmark  function benchmark handle
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_func_destroy(muggle_benchmark_func_t *benchmark);

/**
 * @brief run function benchmark
 *
 * @param benchmark function benchmark handle
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_func_run(muggle_benchmark_func_t *benchmark);

/**
 * @brief generate report
 *
 * @param benchmark function benchmark handle
 * @param name      report case name
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_func_gen_report(muggle_benchmark_func_t *benchmark, const char *name);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_BENCHMARK_TEMPLATE_FUNC_H_ */
