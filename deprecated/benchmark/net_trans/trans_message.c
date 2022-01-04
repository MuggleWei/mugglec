/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "trans_message.h"

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

void sendPkgs(muggle_socket_peer_t *peer)
{
	struct pkg msg;
	genPkgHeader(&msg.header);

	struct timespec ts_start, ts_end;
	timespec_get(&ts_start, TIME_UTC);

	uint32_t idx = 0;
	for (int i = 0; i < TRANS_PKG_ROUND; i++)
	{
		for (int j = 0; j < PKG_PER_ROUND; j++)
		{
			genPkgData((struct pkg_data*)&msg.placeholder, idx++);
			muggle_socket_send(peer->fd, &msg, sizeof(struct pkg_header) + (size_t)msg.header.data_len, 0);
		}

		if (ROUND_INTERVAL_MS > 0)
		{
			muggle_msleep(ROUND_INTERVAL_MS);
		}
	}

	timespec_get(&ts_end, TIME_UTC);
	uint64_t elapsed_ns = (ts_end.tv_sec - ts_start.tv_sec) * 1000000000 + ts_end.tv_nsec - ts_start.tv_nsec;

	MUGGLE_LOG_INFO("send %u pkg completed, use %llu ns", (unsigned int)idx, (unsigned long long)elapsed_ns);

	muggle_msleep(5);
	memset(&msg, 0, sizeof(msg));
	msg.header.msg_type = MSG_TYPE_END;
	muggle_socket_send(peer->fd, &msg, sizeof(struct pkg_header) + (size_t)msg.header.data_len, 0);
	MUGGLE_LOG_INFO("send end pkg");
}
