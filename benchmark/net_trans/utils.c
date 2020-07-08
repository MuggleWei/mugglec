/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "utils.h"

int       g_pkg_cnt                 = 0;
fn_on_pkg g_callbacks[MAX_MSG_TYPE] = {NULL};

muggle_benchmark_block_t *g_blocks = NULL;

/****************** report ******************/
void init_report()
{
	g_blocks = malloc(sizeof(muggle_benchmark_block_t) * TRANS_PKG_ROUND * PKG_PER_ROUND);
	for (int i = 0; i < TRANS_PKG_ROUND * PKG_PER_ROUND; i++)
	{
		memset(&g_blocks[i], 0, sizeof(muggle_benchmark_block_t));
	}
}
void gen_report(const char *name)
{
	MUGGLE_LOG_INFO("recv %d pkgs", g_pkg_cnt);
	gen_benchmark_report(name, g_blocks, g_pkg_cnt);

	free(g_blocks);
}
void gen_benchmark_report(const char *name, muggle_benchmark_block_t *blocks, int cnt)
{
	muggle_benchmark_config_t config;
	strncpy(config.name, name, sizeof(config.name)-1);
	config.loop = TRANS_PKG_ROUND;
	config.cnt_per_loop = PKG_PER_ROUND;
	config.loop_interval_ms = ROUND_INTERVAL_MS;
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
	muggle_benchmark_gen_reports_body(fp, &config, blocks, "sort by idx", cnt, 0, 1, 0);
	muggle_benchmark_gen_reports_body(fp, &config, blocks, "sort by elapsed", cnt, 0, 1, 1);

	fclose(fp);
}

/****************** message callbacks ******************/
void register_callbacks()
{
	g_callbacks[MSG_TYPE_NULL] = NULL;
	g_callbacks[MSG_TYPE_PKG] = on_pkg;
	g_callbacks[MSG_TYPE_END] = on_end;
}

int on_msg(muggle_socket_peer_t *peer, struct pkg *msg)
{
	if (msg->header.msg_type >= MAX_MSG_TYPE)
	{
		MUGGLE_LOG_ERROR("invalid msg_type: %d", (int)msg->header.msg_type);
		return -1;
	}

	if (g_callbacks[msg->header.msg_type])
	{
		return g_callbacks[msg->header.msg_type](peer, msg);
	}

	MUGGLE_LOG_ERROR("callbacks for msg_type %d is null", (int)msg->header.msg_type);
	return -1;
}

int on_pkg(muggle_socket_peer_t *peer, struct pkg *msg)
{
	struct pkg_data *data = (struct pkg_data*)msg->placeholder;

	if (data->idx >= TRANS_PKG_ROUND * PKG_PER_ROUND)
	{
		MUGGLE_LOG_ERROR("message idx[%u] beyond range", (unsigned int)data->idx);
		return -1;
	}

	muggle_benchmark_block_t *block = &g_blocks[g_pkg_cnt++];
	block->ts[0].tv_sec = data->sec;
	block->ts[0].tv_nsec = data->nsec;

	timespec_get(&block->ts[1], TIME_UTC);

	return 0;
}
int on_end(muggle_socket_peer_t *peer, struct pkg *msg)
{
	MUGGLE_LOG_INFO("recv end message");
	return 1;
}
