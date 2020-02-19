#include "echo_serv_handle.h"

int main(int argc, char *argv[])
{
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	if (muggle_socket_lib_init() != 0)
	{
		MUGGLE_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	if (argc < 3)
	{
		MUGGLE_ERROR("usage: %s <IP> <Port> [thread|select|poll|epoll|iocp|kqueue]", argv[0]);
		exit(EXIT_FAILURE);
	}

#if !MUGGLE_PLATFORM_WINDOWS
	// ignore PIPE
	signal(SIGPIPE, SIG_IGN);
#endif

	const char *host = argv[1];
	const char *serv = argv[2];

	// peers
	int cnt_peer = 2;
	muggle_socket_peer_t peers[2];

	// create listen socket
	peers[0].fd = muggle_tcp_listen(host, serv, 512, &peers[0]);
	if (peers[0].fd == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_ERROR("failed create tcp listen for %s:%s", host, serv);
		exit(EXIT_FAILURE);
	}

	// create bind udp
	peers[1].fd = muggle_udp_bind(host, serv, &peers[1]);
	if (peers[1].fd == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_ERROR("failed create udp bind for %s:%s", host, serv);
		exit(EXIT_FAILURE);
	}

	// get socket event loop type
	int event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL;
	if (argc > 3)
	{
		if (strcmp(argv[3], "thread") == 0)
		{
			event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_MULTHREAD;
		}
		else if (strcmp(argv[3], "select") == 0)
		{
			event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT;
		}
		else if (strcmp(argv[3], "poll") == 0)
		{
			event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_POLL;
		}
		else if (strcmp(argv[3], "epoll") == 0)
		{
			event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_EPOLL;
		}
		else if (strcmp(argv[3], "iocp") == 0)
		{
			event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_IOCP;
		}
		else if (strcmp(argv[3], "kqueue") == 0)
		{
			event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_KQUEUE;
		}
		else
		{
			MUGGLE_ERROR("invalid socket event loop type: %s", argv[3]);
			exit(EXIT_FAILURE);
		}
	}

	// fill up event loop input arguments
	const char *hello = "hello echo service";
	muggle_socket_ev_arg_t ev_arg;
	ev_arg.ev_loop_type = event_loop_type;
	ev_arg.hints_max_peer = 1024;
	ev_arg.cnt_peer = cnt_peer;
	ev_arg.peers = peers;
	ev_arg.timeout_ms = 5000;
	ev_arg.datas = (void*)hello;
	ev_arg.on_connect = on_connect;
	ev_arg.on_error = on_error;
	ev_arg.on_message = on_message;
	ev_arg.on_timer = on_timer;

	// event loop
	muggle_socket_event_loop(&ev_arg);

	return 0;
}
