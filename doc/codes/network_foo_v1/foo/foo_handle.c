#include "foo_handle.h"

void foo_on_connect(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *listen_peer,
	muggle_socket_peer_t *peer)
{
	foo_socket_peer_data_t *data = (foo_socket_peer_data_t*)malloc(sizeof(foo_socket_peer_data_t));
	if (data == NULL)
	{
		MUGGLE_LOG_ERROR("failed allocate memory space");
		return;
	}

	memset(data, 0, sizeof(foo_socket_peer_data_t));

	if (!muggle_bytes_buffer_init(&data->bytes_buf, 1024 * 1024))
	{
		MUGGLE_LOG_ERROR("failed init bytes buffer");
		free(data);
		return;
	}

	if (muggle_socket_ntop(
			(struct sockaddr*)&peer->addr,
			data->straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
	{
		snprintf(data->straddr, MUGGLE_SOCKET_ADDR_STRLEN, "unknown:unknown");
	}

	peer->data = data;

	MUGGLE_LOG_INFO("socket connection: addr=%s", data->straddr);
}

void foo_on_message(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer)
{
	if (peer->data == NULL)
	{
		MUGGLE_LOG_ERROR("failed get peer's data");
		return;
	}

	// read bytes into bytes buffer
	foo_ev_data_t *ev_data = (foo_ev_data_t*)ev->datas;
	foo_socket_peer_data_t *data = (foo_socket_peer_data_t*)peer->data;
	muggle_bytes_buffer_t *bytes_buf = &data->bytes_buf;
	while (1)
	{
		void *p = muggle_bytes_buffer_writer_fc(bytes_buf, ev_data->recv_unit_size);
		if (p == NULL)
		{
			MUGGLE_LOG_WARNING("bytes buffer full: %s", data->straddr);
			break;
		}

		int n = muggle_socket_peer_recv(peer, p, ev_data->recv_unit_size, 0);
		if (n > 0)
		{
			muggle_bytes_buffer_writer_move(bytes_buf, n);
		}

		if (n < ev_data->recv_unit_size)
		{
			break;
		}
	}

	// dispatch message
	bool ret = foo_dispatcher_handle(ev, peer);
	if (!ret)
	{
		muggle_socket_peer_close(peer);
	}
}

void foo_on_close(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer)
{
	foo_socket_peer_data_t *data = (foo_socket_peer_data_t*)peer->data;
	if (data == NULL)
	{
		return;
	}

	MUGGLE_LOG_INFO("socket close: addr=%s", data->straddr);

	muggle_bytes_buffer_destroy(&data->bytes_buf);
	free(data);
	peer->data = NULL;
}

void foo_send(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data,
	size_t num_bytes)
{
	muggle_socket_peer_send(peer, data, num_bytes, 0);
}
