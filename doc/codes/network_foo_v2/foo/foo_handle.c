#include "foo_handle.h"

void foo_on_connect(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *listen_peer,
	muggle_socket_peer_t *peer)
{
	// allocate new peer's data
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

void foo_on_error(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer)
{
	if (peer->data == NULL)
	{
		return;
	}

	foo_socket_peer_data_t *data = (foo_socket_peer_data_t*)peer->data;
	MUGGLE_LOG_INFO("socket disconnect: %s", data->straddr);
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

	// destroy peer's data
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
	foo_dispatcher_send(ev, peer, data, (uint32_t)num_bytes);
}
