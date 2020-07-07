/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "trans_message.h"
#include "udp_sender.h"
#include "udp_receiver.h"

int main(int argc, char *argv[])
{
	// init log
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	// init socket library
	if (muggle_socket_lib_init() != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	if (argc < 4)
	{
		MUGGLE_LOG_ERROR("usage: %s <udp-send|udp-recv|tcp-serv|tcp-client> <host> <port>");
		exit(EXIT_FAILURE);
	}

	const char *app_type = argv[1];
	const char *host = argv[2];
	const char *port = argv[3];

	if (strcmp(app_type, "udp-send") == 0)
	{
		run_udp_sender(host, port);
	}
	else if (strcmp(app_type, "udp-recv") == 0)
	{
		run_udp_receiver(host, port);
	}
	else
	{
		MUGGLE_LOG_WARNING("invalid app type: %s", app_type);
	}

	return 0;
}
