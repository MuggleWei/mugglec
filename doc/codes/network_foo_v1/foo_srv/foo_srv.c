#include "foo_srv_handle.h"

int main(int argc, char *argv[])
{
	// init log and socket library
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	if (muggle_socket_lib_init() != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	// init dispatcher
	foo_dispatcher_t dispatcher;
	foo_dispatcher_init(&dispatcher);
	foo_dispather_register(&dispatcher, FOO_MSG_TYPE_REQ_LOGIN, sizeof(foo_msg_req_login_t), on_msg_req_login);
	foo_dispather_register(&dispatcher, FOO_MSG_TYPE_REQ_SUM, FOO_VARIABLE_LEN_MSG, on_msg_req_sum);
	
	// intialize event data
	foo_ev_data_t ev_data;
	memset(&ev_data, 0, sizeof(ev_data));
	ev_data.dispatcher = &dispatcher;
	ev_data.recv_unit_size = 4096;
	ev_data.msg_len_limit = 0;

	// create tcp listen socket peer
	if (argc < 3)
	{
		MUGGLE_LOG_ERROR("usage: %s <IP> <Port|Service>", argv[0]);
		exit(EXIT_FAILURE);
	}
	const char *host = argv[1];
	const char *serv = argv[2];

	muggle_socket_peer_t peer;
	if (muggle_tcp_listen(host, serv, 512, &peer) == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create tcp listen for %s:%s", host, serv);
		exit(EXIT_FAILURE);
	}

	// fill up event loop input arguments
	muggle_socket_event_init_arg_t ev_init_arg;
	memset(&ev_init_arg, 0, sizeof(ev_init_arg));
	ev_init_arg.hints_max_peer = 1024;
	ev_init_arg.cnt_peer = 1;
	ev_init_arg.peers = &peer;
	ev_init_arg.p_peers = NULL;
	ev_init_arg.timeout_ms = 5000;
	ev_init_arg.datas = &ev_data;
	ev_init_arg.on_connect = on_connect;
	ev_init_arg.on_error = on_error;
	ev_init_arg.on_close = on_close;
	ev_init_arg.on_message = on_message;
	ev_init_arg.on_timer = on_timer;

	// event loop
	muggle_socket_event_t ev;
	if (muggle_socket_event_init(&ev_init_arg, &ev) != 0)
	{
		MUGGLE_LOG_ERROR("failed init socket event");
		exit(EXIT_FAILURE);
	}
	muggle_socket_event_loop(&ev);
	
	return 0;
}