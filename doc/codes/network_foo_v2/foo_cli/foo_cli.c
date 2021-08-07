#include "foo_cli_handle.h"
#include "foo/codec/codec_endian.h"
#include "foo/codec/codec_bytes.h"

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
	foo_dispather_register(&dispatcher, FOO_MSG_TYPE_RSP_LOGIN, on_msg_rsp_login);
	foo_dispather_register(&dispatcher, FOO_MSG_TYPE_RSP_SUM, on_msg_rsp_sum);

	// append codec into dispatcher
	foo_codec_endian_t endian_codec;
	foo_codec_endian_init(&endian_codec);

	foo_codec_t bytes_codec;
	foo_codec_bytes_init(&bytes_codec);

	foo_dispatcher_append_codec(&dispatcher, (foo_codec_t*)&endian_codec);
	foo_dispatcher_append_codec(&dispatcher, (foo_codec_t*)&bytes_codec);

	// intialize event data
	foo_ev_data_t ev_data;
	memset(&ev_data, 0, sizeof(ev_data));
	ev_data.dispatcher = &dispatcher;
	ev_data.recv_unit_size = 4096;
	ev_data.msg_len_limit = 0;

	// create tcp connect socket peer
	if (argc < 3)
	{
		MUGGLE_LOG_ERROR("usage: %s <IP> <Port|Service>", argv[0]);
		exit(EXIT_FAILURE);
	}
	const char *host = argv[1];
	const char *serv = argv[2];

	// init rand seed
	srand(time(NULL));

	muggle_socket_peer_t *connect_peer;
	int connect_timeout = 3;
	while (1)
	{
		muggle_socket_peer_t peer;
		muggle_tcp_connect(host, serv, connect_timeout, &peer);

		if (peer.fd == MUGGLE_INVALID_SOCKET)
		{
			MUGGLE_LOG_ERROR("failed connect %s:%s", host, serv);
			muggle_msleep(3000);
			continue;
		}
		ev_data.user_data = (void*)&peer;

		// fill up event loop input arguments
		muggle_socket_event_init_arg_t ev_init_arg;
		memset(&ev_init_arg, 0, sizeof(ev_init_arg));
		ev_init_arg.hints_max_peer = 1024;
		ev_init_arg.cnt_peer = 1;
		ev_init_arg.peers = &peer;
		ev_init_arg.p_peers = &connect_peer;
		ev_init_arg.timeout_ms = 1000;
		ev_init_arg.datas = &ev_data;
		ev_init_arg.on_connect = NULL;
		ev_init_arg.on_error = on_error;
		ev_init_arg.on_close = on_close;
		ev_init_arg.on_message = on_message;
		ev_init_arg.on_timer = on_timer;

		// event loop init
		muggle_socket_event_t ev;
		if (muggle_socket_event_init(&ev_init_arg, &ev) != 0)
		{
			MUGGLE_LOG_ERROR("failed init socket event");
			exit(EXIT_FAILURE);
		}

		// on client connection
		on_connect(&ev, NULL, connect_peer);

		// event loop
		muggle_socket_event_loop(&ev);

		muggle_msleep(3000);
	}

	return 0;
}
