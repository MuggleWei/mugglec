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

	sendPkgs(&udp_peer);
}
