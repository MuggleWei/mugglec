/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "tcp_client.h"
#include "utils.h"

void recv_message(muggle_socket_peer_t *peer, muggle_bytes_buffer_t *bytes_buf)
{
	int read_bytes = 4096;
	while (1)
	{
		void *p = muggle_bytes_buffer_writer_fc(bytes_buf, read_bytes);
		if (p == NULL)
		{
			MUGGLE_LOG_ERROR("bytes buffer full");
			exit(EXIT_FAILURE);
		}

		int n = muggle_socket_peer_recv(peer, p, read_bytes, 0);
		muggle_bytes_buffer_writer_move(bytes_buf, n);

		if (n < read_bytes)
		{
			break;
		}
	}
}

bool parse_message_uncontiguous(muggle_socket_peer_t *peer, muggle_bytes_buffer_t *bytes_buf, int readable)
{
	struct pkg msg;
	if (!muggle_bytes_buffer_fetch(bytes_buf, sizeof(struct pkg_header), &msg.header))
	{
		return false;
	}

	if (readable >= msg.header.data_len + sizeof(struct pkg_header))
	{
		bool ret = muggle_bytes_buffer_read(bytes_buf, msg.header.data_len + sizeof(struct pkg_header), &msg);
		MUGGLE_ASSERT(ret == true);
		if (!ret)
		{
			return false;
		}

		if (on_msg(peer, &msg) != 0)
		{
			muggle_socket_peer_close(peer);
			MUGGLE_LOG_INFO("close peer");
		}

		return true;
	}

	return false;
}

void parse_message(muggle_socket_peer_t *peer, muggle_bytes_buffer_t *bytes_buf)
{
	int readable = 0;
	while (1)
	{
		readable = muggle_bytes_buffer_readable(bytes_buf);
		if (readable < (int)sizeof(struct pkg_header))
		{
			break;
		}

		struct pkg_header *header = muggle_bytes_buffer_reader_fc(bytes_buf, sizeof(struct pkg_header));
		if (header != NULL)
		{
			int len_msg = (int)(header->data_len + (uint32_t)sizeof(struct pkg_header));
			if (len_msg > sizeof(struct pkg))
			{
				MUGGLE_LOG_ERROR("size of data was wrong! msg_len=%d, data_len=%d", len_msg, header->data_len);
				exit(EXIT_FAILURE);
			}

			if (readable < len_msg)
			{
				break;
			}

			struct pkg *msg = muggle_bytes_buffer_reader_fc(bytes_buf, len_msg);
			if (msg != NULL)
			{
				if (on_msg(peer, msg) != 0)
				{
					muggle_socket_peer_close(peer);
					MUGGLE_LOG_INFO("close peer");
				}

				muggle_bytes_buffer_reader_move(bytes_buf, len_msg);
			}
			else
			{
				if (!parse_message_uncontiguous(peer, bytes_buf, readable))
				{
					break;
				}
			}
		}
		else
		{
			if (!parse_message_uncontiguous(peer, bytes_buf, readable))
			{
				break;
			}
		}
	}
}

static void tcp_client_on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	muggle_bytes_buffer_t *bytes_buf = (muggle_bytes_buffer_t*)peer->data;

	// read message into bytes buffer
	recv_message(peer, bytes_buf);

	// parse message
	parse_message(peer, bytes_buf);
}

static void tcp_client_on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	muggle_socket_event_loop_exit(ev);
}

void run_tcp_client(const char *host, const char *port)
{
	// init bytes buffer
	muggle_bytes_buffer_t bytes_buf;
	if (!muggle_bytes_buffer_init(&bytes_buf, 1024 * 1024 * 16))
	{
		MUGGLE_LOG_ERROR("failed init bytes buf");
		exit(EXIT_FAILURE);
	}

	// create tcp connect socket
	muggle_socket_peer_t tcp_peer;
	tcp_peer.fd = muggle_tcp_connect(host, port, 3, &tcp_peer);
	if (tcp_peer.fd == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed connect %s:%s", host, port);
		exit(EXIT_FAILURE);
	}
	tcp_peer.data = &bytes_buf;

	// set TCP_NODELAY
	int enable = 1;
	setsockopt(tcp_peer.fd, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));

	// fill up event loop input arguments
	muggle_socket_event_init_arg_t ev_init_arg;
	memset(&ev_init_arg, 0, sizeof(ev_init_arg));
	ev_init_arg.ev_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL;
	ev_init_arg.cnt_peer = 1;
	ev_init_arg.peers = &tcp_peer;
	ev_init_arg.timeout_ms = -1;
	ev_init_arg.on_message = tcp_client_on_message;
	ev_init_arg.on_error = tcp_client_on_error;

	// init benchmark report
	init_report();

	// register callbacks
	register_callbacks();

	// event loop
	muggle_socket_event_t ev;
	if (muggle_socket_event_init(&ev_init_arg, &ev) != 0)
	{
		MUGGLE_LOG_ERROR("failed init socket event");
		exit(EXIT_FAILURE);
	}
	muggle_socket_event_loop(&ev);

	// destroy bytes buffer
	muggle_bytes_buffer_destroy(&bytes_buf);

	// generate benchmark report
	gen_report("tcp_latency");
}
