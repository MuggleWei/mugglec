/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "trans_runner.h"

muggle_thread_ret_t write_thread(void *p_arg)
{
	struct write_thread_args *arg = (struct write_thread_args*)p_arg;

	muggle_msleep(1);

	fn_trans_write trans_fn = arg->fn;
	void *trans_obj = arg->trans_obj;
	for (int i = 0; i < arg->num_block; i++)
	{
		timespec_get(&arg->blocks[i].ts[0], TIME_UTC);
		while (trans_fn(trans_obj, &arg->blocks[i]) != 0)
		{
			continue;
		}
		timespec_get(&arg->blocks[i].ts[1], TIME_UTC);
	}

	trans_fn(trans_obj, NULL);

	return 0;
}

void init_blocks(struct write_thread_args *args, muggle_benchmark_block_t *blocks, int num_thread, int msg_per_write)
{
	for (int i = 0; i < num_thread; i++)
	{
		memset(&args[i], 0, sizeof(struct write_thread_args));
		args[i].blocks = blocks + i * msg_per_write;
		args[i].num_block = msg_per_write;
	}

	int total_msg_num = num_thread * msg_per_write;
	for (int i = 0; i < total_msg_num; i++)
	{
		memset(&blocks[i], 0, sizeof(muggle_benchmark_block_t));
		blocks[i].idx = i;
	}
}

void run_thread_trans_benchmark(struct write_thread_args *args, int num_thread, fn_trans_read fn_read)
{
	muggle_thread_t *threads = (muggle_thread_t*)malloc(num_thread * sizeof(muggle_thread_t));
	for (int i = 0; i < num_thread; i++)
	{
		muggle_thread_create(&threads[i], write_thread, &args[i]);
	}

	int recv_null = 0;
	void *trans_obj = args[0].trans_obj;

	int total_recv = 0;
	while (1)
	{
		void* data = fn_read(trans_obj);
		if (data)
		{
			muggle_benchmark_block_t *block = (muggle_benchmark_block_t*)data;
			timespec_get(&block->ts[2], TIME_UTC);
			total_recv++;
		}
		else
		{
			recv_null++;
			if (recv_null == num_thread)
			{
				break;
			}
		}
	}

	MUGGLE_LOG_INFO("recv total message: %d", total_recv);

	for (int i = 0; i < num_thread; i++)
	{
		muggle_thread_join(&threads[i]);
	}

	free(threads);
}


/****************** report ******************/
void gen_benchmark_report(const char *name, muggle_benchmark_block_t *blocks, int cnt)
{
	muggle_benchmark_config_t config;
	strncpy(config.name, name, sizeof(config.name)-1);
	config.loop = 1;
	config.cnt_per_loop = cnt;
	config.loop_interval_ms = 0;
	config.report_step = 10;

	char file_name[128];
	snprintf(file_name, sizeof(file_name)-1, "benchmark_%s.csv", config.name);
	FILE *fp = fopen(file_name, "wb");
	if (fp == NULL)
	{
		printf("failed open file: %s\n", file_name);
		exit(1);
	}

	muggle_benchmark_gen_reports_head(fp, &config);
	muggle_benchmark_gen_reports_body(fp, &config, blocks, "w sort by idx", cnt, 0, 1, 0);
	muggle_benchmark_gen_reports_body(fp, &config, blocks, "w sort by elapsed", cnt, 0, 1, 1);
	muggle_benchmark_gen_reports_body(fp, &config, blocks, "wr sort by idx", cnt, 0, 2, 0);
	muggle_benchmark_gen_reports_body(fp, &config, blocks, "wr sort by elapsed", cnt, 0, 2, 1);

	fclose(fp);
}
