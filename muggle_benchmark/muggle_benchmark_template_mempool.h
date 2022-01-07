/******************************************************************************
 *  @file         muggle_benchmark_template_mempool.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-01-04
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec benchmark memory pool template
 *****************************************************************************/

#ifndef MUGGLE_C_BENCHMARK_TEMPLATE_MEMPOOL_H_
#define MUGGLE_C_BENCHMARK_TEMPLATE_MEMPOOL_H_

#include "muggle_benchmark/muggle_benchmark_macro.h"
#include "muggle_benchmark/muggle_benchmark_config.h"
#include "muggle_benchmark/muggle_benchmark_handle.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_ALLOC,
	MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_ALLOC,
	MUGGLE_BENCHMARK_MEMPOOL_ACTION_BEFORE_FREE,
	MUGGLE_BENCHMARK_MEMPOOL_ACTION_AFTER_FREE,
	MAX_MUGGLE_BENCHMARK_MEMPOOL_ACTION,
};

/**
 * @brief function prototype of memory pool allocate
 *
 * @param pool        memory pool
 * @param block_sizez allocate block size
 *
 * @return data pointer
 */
typedef void* (*fn_muggle_benchmark_mempool_alloc)(void *pool, size_t block_size);

/**
 * @brief function prototype of memory pool free
 *
 * @param pool  memory pool
 * @param data  data need to free
 */
typedef void (*fn_muggle_benchmark_mempool_free)(void *pool, void *data);

typedef struct muggle_benchmark_mempool
{
	muggle_benchmark_config_t         *config;            //!< benchmark config
	muggle_benchmark_handle_t         handle;             //!< benchmark handle
	int                               support_mul_thread; //!< support multiple thread
	void                              *pool;              //!< memory pool
	fn_muggle_benchmark_mempool_alloc fn_alloc;          //!< allocate function
	fn_muggle_benchmark_mempool_free  fn_free;           //!< freeate function
} muggle_benchmark_mempool_t;

/**
 * @brief initialize memory pool benchmark handle
 *
 * @param benchmark           memory pool benchmark handle
 * @param config              benchmark config
 * @param support_mul_thread  support multiple thread
 * @param pool                memory pool
 * @param fn_alloc            allocate function
 * @param fn_free             free function
 *
 * @return 
 *     0 - success
 *     otherwise - failed
 */
MUGGLE_BENCHMARK_EXPORT
int muggle_benchmark_mempool_init(
	muggle_benchmark_mempool_t *benchmark,
	muggle_benchmark_config_t *config,
	int support_mul_thread,
	void *pool,
	fn_muggle_benchmark_mempool_alloc fn_alloc,
	fn_muggle_benchmark_mempool_free  fn_free);

/**
 * @brief destroy memory pool benchmark handle
 *
 * @param benchmark memory pool benchmark handle
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_mempool_destroy(muggle_benchmark_mempool_t *benchmark);

/**
 * @brief run memory pool benchmark
 *
 * @param benchmark memory pool benchmark handle
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_mempool_run(muggle_benchmark_mempool_t *benchmark);

/**
 * @brief generate report
 *
 * @param benchmark memory pool benchmark handle
 * @param name      report case name
 */
MUGGLE_BENCHMARK_EXPORT
void muggle_benchmark_mempool_gen_report(muggle_benchmark_mempool_t *benchmark, const char *name);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_BENCHMARK_MEMPOOL_H_ */
