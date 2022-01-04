/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "udp_receiver.h"
#include "utils.h"

void run_udp_receiver(const char *host, const char *port)
{
	// init benchmark report
	init_report();

	// register callbacks
	register_callbacks();

	muggle_socket_peer_t udp_peer;
	udp_peer.fd = muggle_udp_bind(host, port, &udp_peer);
	if (udp_peer.fd == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create udp bind for %s:%s", host, port);
		exit(EXIT_FAILURE);
	}

	char buf[65536];
	while (1)
	{
		int n = recv(udp_peer.fd, buf, sizeof(buf), 0);
		if (n > 0)
		{
			if (on_msg(NULL, (struct pkg*)buf) != 0)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	// generate benchmark report
	gen_report("udp_latency");
}
