#include "net_str_client_handle.h"

struct client_event_data
{
	muggle_ring_buffer_t      *ring;
	muggle_sowr_memory_pool_t *peer_event_sowr_pool;
	muggle_sowr_memory_pool_t *text_sowr_pool;
};

void on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	struct client_event_data *ev_data = ev->datas;
	muggle_ring_buffer_t *ring = (muggle_ring_buffer_t*)ev_data->ring;
	struct message_peer_event *msg = (struct message_peer_event*)muggle_sowr_memory_pool_alloc(ev_data->peer_event_sowr_pool);
	msg->msg_type = MSG_TYPE_PEER_DISCONNECT;
	msg->peer = peer;
	muggle_ring_buffer_write(ring, msg);

	muggle_socket_event_loop_exit(ev);
}
void on_close(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
	if (muggle_socket_ntop((struct sockaddr*)&peer->addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
	{
		snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "unknown:unknown");
	}
	MUGGLE_LOG_WARNING("%s closing soon, safe to delete user data here", straddr);
}
void on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	struct client_event_data *ev_data = ev->datas;
	muggle_ring_buffer_t *ring = (muggle_ring_buffer_t*)ev_data->ring;
	struct message_text *msg = (struct message_text*)muggle_sowr_memory_pool_alloc(ev_data->text_sowr_pool);
	msg->msg_type = MSG_TYPE_PEER_RECV;
	int n = muggle_socket_peer_recv(peer, msg->buf, sizeof(msg->buf), 0);
	if (n > 0)
	{
		msg->buf[n] = '\0';
		muggle_ring_buffer_write(ring, msg);
	}
	// don't free message in this thread, let message override automic
	// else
	// {
	// 	muggle_sowr_memory_pool_free(msg);
	// }
}

muggle_thread_ret_t thread_socket_event(void *arg)
{
	struct client_thread_arg *th_arg = (struct client_thread_arg*)arg;

	// get message queue(ring buffer)
	muggle_ring_buffer_t *ring = (muggle_ring_buffer_t*)th_arg->ring;

	// get message pool
	muggle_sowr_memory_pool_t peer_event_sowr_pool;
	muggle_sowr_memory_pool_init(&peer_event_sowr_pool, 16, sizeof(struct message_peer_event));

	muggle_sowr_memory_pool_t text_sowr_pool;
	muggle_sowr_memory_pool_init(&text_sowr_pool, 16, sizeof(struct message_text));

	// event data
	struct client_event_data ev_data;
	ev_data.ring = ring;
	ev_data.peer_event_sowr_pool = &peer_event_sowr_pool;
	ev_data.text_sowr_pool = &text_sowr_pool;

	while (1)
	{
		// create socket
		muggle_socket_peer_t peer;
		muggle_socket_t ret_fd;
		if (strcmp(th_arg->socket_type, "tcp") == 0)
		{
			ret_fd = muggle_tcp_connect(th_arg->host, th_arg->serv, 3, &peer);
		}
		else if (strcmp(th_arg->socket_type, "udp") == 0)
		{
			ret_fd = muggle_udp_connect(th_arg->host, th_arg->serv, &peer);
		}
		else
		{
			MUGGLE_LOG_ERROR("invalid socket peer type: %s", th_arg->socket_type);
			exit(EXIT_FAILURE);
		}

		if (ret_fd == MUGGLE_INVALID_SOCKET)
		{
			MUGGLE_LOG_ERROR("%s failed connect: %s:%s", th_arg->socket_type, th_arg->host, th_arg->serv);
			muggle_msleep(3000);
			MUGGLE_LOG_INFO("reconnect...");
			continue;
		}

		// fill up event loop input arguments
		muggle_socket_peer_t *p_peer = NULL;

		muggle_socket_event_init_arg_t ev_init_arg;
		memset(&ev_init_arg, 0, sizeof(ev_init_arg));
		ev_init_arg.ev_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL;
		ev_init_arg.hints_max_peer = 1024;
		ev_init_arg.cnt_peer = 1;
		ev_init_arg.peers = &peer;
		ev_init_arg.p_peers = &p_peer;
		ev_init_arg.timeout_ms = -1;
		ev_init_arg.datas = NULL;
		ev_init_arg.on_error = on_error;
		ev_init_arg.on_close = on_close;
		ev_init_arg.on_message = on_message;
		ev_init_arg.datas = &ev_data;

		// init event loop
		muggle_socket_event_t ev;
		if (muggle_socket_event_init(&ev_init_arg, &ev) != 0)
		{
			MUGGLE_LOG_ERROR("failed init socket event");
			exit(EXIT_FAILURE);
		}

		// retain peer
		muggle_socket_peer_retain(p_peer);
		struct message_peer_event *msg = (struct message_peer_event*)muggle_sowr_memory_pool_alloc(&peer_event_sowr_pool);
		msg->msg_type = MSG_TYPE_PEER_CONNECT;
		msg->peer = p_peer;
		muggle_ring_buffer_write(ring, msg);

		// run event loop
		muggle_socket_event_loop(&ev);

		// sleep 3s
		muggle_msleep(3000);
		MUGGLE_LOG_INFO("reconnect...");
	}

	// wait for all message consumed, destroy sowr pool
	while (!muggle_sowr_memory_pool_is_all_free(&peer_event_sowr_pool))
	{
		muggle_msleep(100);
	}
	muggle_sowr_memory_pool_destroy(&peer_event_sowr_pool);

	while (!muggle_sowr_memory_pool_is_all_free(&text_sowr_pool))
	{
		muggle_msleep(100);
	}
	muggle_sowr_memory_pool_destroy(&text_sowr_pool);
}
