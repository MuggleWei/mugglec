/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "trans_message.h"

muggle_benchmark_block_t *g_blocks = NULL;

void genPkgHeader(struct pkg_header *header)
{
	int x = 1;
	int little_endian = (*(char*)&x == (char)1) ? 1 : 0;

	header->little_endian = little_endian;
	header->msg_type = MSG_TYPE_PKG;
	header->data_len = sizeof(struct pkg_data);
}

void genPkgData(struct pkg_data *data, uint32_t idx)
{
	data->idx = idx;

	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	data->sec = (uint64_t)ts.tv_sec;
	data->nsec = (uint64_t)ts.tv_nsec;
}
