#include "dispatcher.h"
#include "foo_handle.h"

void foo_dispatcher_init(foo_dispatcher_t *dispatcher)
{
	foo_callback_t *callbacks = dispatcher->callbacks;
	for (int i = 0; i < FOO_MSG_TYPE_MAX; i++)
	{
		memset(callbacks + i, 0, sizeof(foo_callback_t));
	}
}

bool foo_dispather_register(
	foo_dispatcher_t *dispatcher,
	uint32_t msg_type,
	int32_t msg_len,
	foo_callback_func callback)
{
	if (msg_type >= FOO_MSG_TYPE_MAX)
	{
		MUGGLE_LOG_ERROR(
			"failed register callback, message type beyond max: msg_type=%lu",
			(unsigned long)msg_type);
		return false;
	}

	foo_callback_t *callbacks = dispatcher->callbacks;
	foo_callback_t *cb = callbacks + msg_type;
	if (cb->msg_type != 0)
	{
		MUGGLE_LOG_ERROR(
			"failed register callback, duplicated message type: msg_type=%lu",
			(unsigned long)msg_type);
		return false;
	}

	cb->msg_type = msg_type;
	cb->msg_len = msg_len;
	cb->cb_func = callback;

	return true;
}

static bool foo_check_header(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	foo_msg_header_t *p_header,
	foo_dispatcher_t *dispatcher,
	const char *straddr)
{
	// check message type
	if (p_header->msg_type >= FOO_MSG_TYPE_MAX)
	{
		MUGGLE_LOG_ERROR("invalide message type: addr=%s, msg_type=%lu",
			straddr, (unsigned long)p_header->msg_type);
		return false;
	}

	// check callback
	foo_callback_t *cb = &dispatcher->callbacks[p_header->msg_type];
	if (cb == NULL)
	{
		MUGGLE_LOG_ERROR("message type without callback: addr=%s, msg_type=%lu",
			straddr, (unsigned long)p_header->msg_type);
		return false;
	}

	// check message length
	uint32_t total_bytes = (uint32_t)sizeof(foo_msg_header_t) + p_header->body_len;
	if (cb->msg_len >= 0 && total_bytes != cb->msg_len)
	{
		MUGGLE_LOG_ERROR(
			"invalid message length: addr=%s, msg_type=%lu, recv_len=%lu, expect_len=%lu",
			straddr, (unsigned long)p_header->msg_type,
			(unsigned long)total_bytes, (unsigned long)cb->msg_len);
		return false;
	}

	return true;
}

bool foo_dispatcher_handle(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer)
{
	foo_ev_data_t *ev_data = (foo_ev_data_t*)peer->data;
	foo_dispatcher_t *dispatcher = ev_data->dispatcher;

	foo_socket_peer_data_t *data = (foo_socket_peer_data_t*)peer->data;
	muggle_bytes_buffer_t *bytes_buf = &data->bytes_buf;

	foo_msg_header_t msg_header;
	while (1)
	{
		if (!muggle_bytes_buffer_fetch(bytes_buf, (int)sizeof(msg_header), &msg_header))
		{
			// readable bytes less than sizeof message header
			break;
		}

		// check message header
		if (!foo_check_header(ev, peer, &msg_header, dispatcher, data->straddr))
		{
			return false;
		}

		// check readable
		foo_callback_t *cb = &dispatcher->callbacks[msg_header.msg_type];
		int readable = muggle_bytes_buffer_readable(bytes_buf);
		if ((uint32_t)readable < msg_header.body_len + (uint32_t)sizeof(foo_msg_header_t))
		{
			break;
		}

		// handle message
		foo_msg_header_t *p_header =
			(foo_msg_header_t*)muggle_bytes_buffer_reader_fc(bytes_buf, (int)cb->msg_len);
		if (p_header == NULL)
		{
			// discontinuous memory
			void *buf = malloc(cb->msg_len);

			muggle_bytes_buffer_read(bytes_buf, (int)cb->msg_len, buf);
			cb->cb_func(ev, peer, buf);

			free(buf);
		}
		else
		{
			// continuous memory
			cb->cb_func(ev, peer, (void*)p_header);

			muggle_bytes_buffer_reader_move(bytes_buf, (int)cb->msg_len);
		}
	}

	return true;
}
