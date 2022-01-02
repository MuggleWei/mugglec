/******************************************************************************
 *  @file         muggle_benchmark_thread_trans.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-01-02
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec benchmark thread transfer template
 *****************************************************************************/

#ifndef MUGGLE_C_BENCHMARK_THREAD_TRANS_H_
#define MUGGLE_C_BENCHMARK_THREAD_TRANS_H_

#include "muggle_benchmark/muggle_benchmark_macro.h"
#include "muggle_benchmark/muggle_benchmark_config.h"
#include "muggle_benchmark/muggle_benchmark_handle.h"

EXTERN_C_BEGIN

/**
 * @brief basic thread transfer message
 */
typedef struct muggle_benchmark_thread_message
{
	uint64_t id;
} muggle_benchmark_thread_message_t;

enum
{
	MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_BEG,
	MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_WRITE_END,
	MUGGLE_BENCHMARK_THREAD_TRANS_ACTION_READ,
	MAX_MUGGLE_BENCHMARK_THREAD_TRANS,
};

/**
 * @brief function prototype of thread transfer write
 *
 * @param user_args  user customized arguments
 * @param data       transfer data
 *
 * @return 
 */
typedef int (*fn_muggle_benchmark_thread_trans_write)(void *user_args, void *data);

/**
 * @brief function prototype of thread transfer read
 *
 * @param user_args  user customized arguments
 *
 * @return transfer data
 */
typedef void* (*fn_muggle_benchmark_thread_trans_read)(void *user_args);

/**
 * @brief function prototype of thread transfer producer complete
 */
typedef void (*fn_muggle_benchmark_thread_trans_producer_completed)(muggle_benchmark_config_t *config, void *user_args);

/**
 * @brief thread transfer benchmark handle
 */
typedef struct muggle_benchmark_thread_trans
{
	muggle_benchmark_config_t              *config;    //!< benchmark config
	muggle_benchmark_handle_t              handle;     //!< benchmark handle
	void                                   *user_args; //!< thread transfer user arguments
	fn_muggle_benchmark_thread_trans_write fn_write;   //!< write function
	fn_muggle_benchmark_thread_trans_read  fn_read;    //!< read function

	fn_muggle_benchmark_thread_trans_producer_completed	fn_completed; //!< producer completed
} muggle_benchmark_thread_trans_t;

/**
 * @brief initialize thread transfer benchmark handle
 *
 * @param benchmark    thread transfer benchmark handle
 * @param config       benchmark config
 * @param user_args    user customized arguments
 * @param fn_write     write function
 * @param fn_read      read function
 * @param fn_completed producer completed
 *
 * @return 
 */
MUGGLE_BENCHMARK_EXPORT
int muggle_benchmark_thread_trans_init(
	muggle_benchmark_thread_trans_t *benchmark,
	muggle_benchmark_config_t *config,
	void *user_args,
	fn_muggle_benchmark_thread_trans_write fn_write,
	fn_muggle_benchmark_thread_trans_read fn_read,
	fn_muggle_benchmark_thread_trans_producer_completed	fn_completed);

/**
 * @brief destroy thread transfer benchmark handle
 *
 * @param benchmark  thread transfer benchmark handle
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_thread_trans_destroy(muggle_benchmark_thread_trans_t *benchmark);

/**
 * @brief run thread transfer benchmark
 *
 * @param benchmark  thread transfer benchmark handle
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_thread_trans_run(muggle_benchmark_thread_trans_t *benchmark);

/**
 * @brief generate report
 *
 * @param benchmark thread transfer benchmark handle
 * @param name      report case name
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_thread_trans_gen_report(muggle_benchmark_thread_trans_t *benchmark, const char *name);

EXTERN_C_END

#endif // !MUGGLE_C_BENCHMARK_THREAD_TRANS_H_
