#include "time_client_handle.h"

int main(int argc, char *argv[])
{
	// initialize log
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_ERROR("failed init log");
		exit(EXIT_FAILURE);
	}

	if (argc != 4)
	{
		MUGGLE_ERROR("usage: %s <host> <port> <tcp|udp|mcast>", argv[0]);
		exit(EXIT_FAILURE);
	}

	// initialize socket library
	muggle_socket_lib_init();

#if !MUGGLE_PLATFORM_WINDOWS
	// ignore PIPE
	signal(SIGPIPE, SIG_IGN);
#endif

	// reconnect 3 times
	int tcp_contiguous_failed = 0;
	while (1)
	{
		// create peer
		muggle_socket_peer_t peer;
		if (strcmp(argv[3], "tcp") == 0)
		{
			peer.fd = muggle_tcp_connect(argv[1], argv[2], 3, &peer);
		}
		else if (strcmp(argv[3], "udp") == 0)
		{
			peer.fd = muggle_udp_bind(argv[1], argv[2], &peer);
		}
		else if (strcmp(argv[3], "mcast") == 0)
		{
			peer.fd = muggle_mcast_join(argv[1], argv[2], NULL, NULL, &peer);
		}
		else
		{
			MUGGLE_ERROR("invalid socket peer type: %s", argv[3]);
			exit(EXIT_FAILURE);
		}

		if (peer.fd == MUGGLE_INVALID_SOCKET)
		{
			if (strcmp(argv[3], "tcp") == 0)
			{
				MUGGLE_ERROR("failed connect %s:%s", argv[1], argv[2]);
				if (++tcp_contiguous_failed >= 3)
				{
					exit(EXIT_FAILURE);
				}
				else
				{
					muggle_msleep(3000);
					MUGGLE_INFO("try to reconnect %s:%s", argv[1], argv[2]);
				}
				continue;
			}
			else
			{
				MUGGLE_ERROR("%s failed create socket for: %s:%s", argv[3], argv[1], argv[2]);
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(argv[3], "tcp") == 0)
		{
			MUGGLE_INFO("%s success connect %s:%s", argv[3], argv[1], argv[2]);
		}

		// reset tcp contiguous connection failed count
		tcp_contiguous_failed = 0;

		// create bytes buffer for socket
		muggle_bytes_buffer_t bytes_buf;
		muggle_bytes_buffer_init(&bytes_buf, 16 * 1024 * 1024);
		peer.data = &bytes_buf;

		// fill up event loop input arguments
#if MUGGLE_PLATFORM_LINUX
		int event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_EPOLL;
#else
		int event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT;
#endif

		muggle_socket_ev_arg_t ev_arg;
		ev_arg.ev_loop_type = event_loop_type;
		ev_arg.hints_max_peer = 1024;
		ev_arg.cnt_peer = 1;
		ev_arg.peers = &peer;
		ev_arg.timeout_ms = -1;
		ev_arg.datas = NULL;
		ev_arg.on_connect = NULL;
		ev_arg.on_error = on_error;
		ev_arg.on_message = on_message;
		ev_arg.on_timer = NULL;

		// event loop
		muggle_socket_event_loop(&ev_arg);
	}

	return 0;
}