/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "alloc_free_runner.h"

static muggle_thread_ret_t alloc_thread(void *p_arg)
{
	struct alloc_free_args *args = (struct alloc_free_args*)p_arg;

	muggle_benchmark_block_t *blocks = args->blocks;
	void **data_container = args->data_container;

	fn_alloc cb_alloc = args->cb_alloc;
	void *allocator = args->allocator;
	muggle_atomic_int *alloc_cursor = &args->alloc_cursor;
	size_t data_size = args->data_size;

	muggle_atomic_int idx;
	void *data = NULL;
	while (1)
	{
		idx = muggle_atomic_fetch_add(alloc_cursor, 1, muggle_memory_order_relaxed);
		if (idx >= args->cnt_blocks)
		{
			break;
		}

		while (1)
		{
			timespec_get(&blocks[idx].ts[0], TIME_UTC);
			data = cb_alloc(allocator, data_size);
			timespec_get(&blocks[idx].ts[1], TIME_UTC);

			if (data)
			{
				data_container[idx] = data;
				break;
			}
			else
			{
				MUGGLE_LOG_WARNING("failed allocate memory for idx[%d], retry", idx);
				muggle_thread_yield();
			}
		};
	}

	MUGGLE_LOG_INFO("allocate thread exit");

	return 0;
}

static muggle_thread_ret_t free_thread(void *p_arg)
{
	struct alloc_free_args *args = (struct alloc_free_args*)p_arg;

	muggle_benchmark_block_t *blocks = args->blocks;
	void **data_container = args->data_container;

	fn_free cb_free = args->cb_free;
	void *allocator = args->allocator;
	muggle_atomic_int *free_cursor = &args->free_cursor;
	muggle_atomic_int *alloc_cursor = &args->alloc_cursor;

	muggle_atomic_int idx;
	void *data = NULL;
	while (1)
	{
		idx = muggle_atomic_load(free_cursor, muggle_memory_order_relaxed);
		if (idx >= args->cnt_blocks)
		{
			break;
		}

		if (idx < muggle_atomic_load(alloc_cursor, muggle_memory_order_relaxed))
		{
			if (muggle_atomic_cmp_exch_strong(free_cursor, &idx, idx+1, muggle_memory_order_relaxed))
			{
				timespec_get(&blocks[idx].ts[2], TIME_UTC);
				cb_free(allocator, data_container[idx]);
				timespec_get(&blocks[idx].ts[3], TIME_UTC);
			}
		}
		else
		{
			muggle_thread_yield();
		}
	}

	MUGGLE_LOG_INFO("free thread exit");

	return 0;
}

void run_alloc_free_benchmark(const char *name, struct alloc_free_args *args)
{
	// check args
	if (args->cfg == NULL ||
		args->cnt_blocks <= 0 ||
		args->blocks == NULL ||
		args->data_container == NULL ||
		args->data_size <= 0 ||
		args->cb_alloc == NULL ||
		args->cb_free == NULL)
	{
		MUGGLE_LOG_ERROR("invalid parameter");
		return;
	}

	// init blocks and data container
	for (int i = 0; i < args->cnt_blocks; i++)
	{
		memset(&args->blocks[i], 0, sizeof(muggle_benchmark_block_t));
		args->blocks[i].idx = i;

		args->data_container[i] = NULL;
	}

	// init alloc and free cursor
	args->alloc_cursor = 0;
	args->free_cursor = 0;

	if (args->num_alloc_threads == 0 || args->num_free_threads == 0)
	{
		MUGGLE_LOG_INFO("start %s allocate", name);
		alloc_thread((void*)args);
		MUGGLE_LOG_INFO("%s allocate completed", name);

		MUGGLE_LOG_INFO("start %s free", name);
		free_thread((void*)args);
		MUGGLE_LOG_INFO("%s free completed", name);
	}
	else
	{
	}

	// gen report
	MUGGLE_LOG_INFO("start gen report for benchmark %s", name);
	gen_benchmark_report(name, args->blocks, args->cfg, args->cnt_blocks);
	MUGGLE_LOG_INFO("gen report completed for benchmark %s", name);
}

void gen_benchmark_report(const char *name, muggle_benchmark_block_t *blocks, muggle_benchmark_config_t *cfg, int cnt)
{
	strncpy(cfg->name, name, sizeof(cfg->name)-1);

	char file_name[128];
	snprintf(file_name, sizeof(file_name)-1, "benchmark_%s.csv", cfg->name);
	FILE *fp = fopen(file_name, "wb");
	if (fp == NULL)
	{
		printf("failed open file: %s\n", file_name);
		exit(1);
	}

	muggle_benchmark_gen_reports_head(fp, cfg);
	muggle_benchmark_gen_reports_body(fp, cfg, blocks, "alloc sort by idx", cnt, 0, 1, 0);
	muggle_benchmark_gen_reports_body(fp, cfg, blocks, "alloc sort by elapsed", cnt, 0, 1, 1);
	muggle_benchmark_gen_reports_body(fp, cfg, blocks, "free sort by idx", cnt, 2, 3, 0);
	muggle_benchmark_gen_reports_body(fp, cfg, blocks, "free sort by elapsed", cnt, 2, 3, 1);

	fclose(fp);
}
