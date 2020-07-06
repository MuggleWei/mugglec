/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "udp_sender.h"

void run_udp_sender(const char *host, const char *port)
{
	muggle_socket_peer_t udp_peer;
	udp_peer.fd = muggle_udp_connect(host, port, &udp_peer);
	if (udp_peer.fd == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed connect udp sender target");
		exit(EXIT_FAILURE);
	}

	struct pkg msg;
	genPkgHeader(&msg.header);

	struct timespec ts_start, ts_end;
	timespec_get(&ts_start, TIME_UTC);

	uint32_t idx = 0;
	for (int i = 0; i < TRANS_PKG_ROUND; i++)
	{
		for (int j = 0; j < PKG_PER_ROUND; j++)
		{
			genPkgData(&msg.data, idx++);
			send(udp_peer.fd, &msg, sizeof(struct pkg_header) + msg.header.data_len, 0);
		}
	}

	timespec_get(&ts_end, TIME_UTC);
	uint64_t elapsed_ns = (ts_end.tv_sec - ts_start.tv_sec) * 1000000000 + ts_end.tv_nsec - ts_start.tv_nsec;

	MUGGLE_LOG_INFO("send pkg completed, use %llu ns", (unsigned long long)elapsed_ns);

	muggle_msleep(5);
	memset(&msg, 0, sizeof(msg));
	msg.header.msg_type = MSG_TYPE_END;
	send(udp_peer.fd, &msg, sizeof(struct pkg_header) + msg.header.data_len, 0);
	MUGGLE_LOG_INFO("send end pkg");
}
