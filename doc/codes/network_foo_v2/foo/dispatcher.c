#include "dispatcher.h"
#include "foo_handle.h"

void foo_dispatcher_init(foo_dispatcher_t *dispatcher)
{
	foo_callback_t *callbacks = dispatcher->callbacks;
	for (int i = 0; i < FOO_MSG_TYPE_MAX; i++)
	{
		memset(callbacks + i, 0, sizeof(foo_callback_t));
	}

	dispatcher->codec_list_head = NULL;
	dispatcher->codec_list_tail = NULL;
}

bool foo_dispather_register(
	foo_dispatcher_t *dispatcher,
	uint32_t msg_type,
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
	cb->cb_func = callback;

	return true;
}

void foo_dispatcher_append_codec(
	foo_dispatcher_t *dispatcher,
	foo_codec_t *codec)
{
	if (dispatcher->codec_list_tail == NULL)
	{
		dispatcher->codec_list_head = codec;
		dispatcher->codec_list_tail = codec;
		codec->next = NULL;
		codec->prev = NULL;
	}
	else
	{
		dispatcher->codec_list_tail->next = codec;
		codec->prev = dispatcher->codec_list_tail;
		dispatcher->codec_list_tail = codec;
	}
}

bool foo_dispatcher_handle(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer)
{
	foo_ev_data_t *ev_data = (foo_ev_data_t*)ev->datas;
	foo_dispatcher_t *dispatcher = ev_data->dispatcher;

	foo_codec_t *codec = dispatcher->codec_list_tail;

	return foo_dispatcher_decode_handle(codec, ev, peer, NULL, 0);
}

bool foo_dispatcher_send(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len)
{
	foo_ev_data_t *ev_data = (foo_ev_data_t*)ev->datas;
	foo_dispatcher_t *dispatcher = ev_data->dispatcher;

	foo_codec_t *codec = dispatcher->codec_list_head;

	return foo_dispatcher_encode_handle(codec, ev, peer, data, data_len);
}

bool foo_dispatcher_encode_handle(
	foo_codec_t *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len)
{
	if (codec)
	{
		return codec->encode(codec, ev, peer, data, data_len);
	}
	else
	{
		muggle_socket_peer_send(peer, data, data_len, 0);
	}

	return true;
}

bool foo_dispatcher_decode_handle(
	foo_codec_t *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len)
{
	if (codec)
	{
		return codec->decode(codec, ev, peer, data, data_len);
	}
	else
	{
		foo_ev_data_t *ev_data = (foo_ev_data_t*)ev->datas;
		foo_dispatcher_t *dispatcher = ev_data->dispatcher;

		foo_msg_header_t *p_header = (foo_msg_header_t*)data;
		foo_callback_t *cb = &dispatcher->callbacks[p_header->msg_type];
		if (cb->cb_func)
		{
			cb->cb_func(ev, peer, data);
		}
		else
		{
			return false;
		}
	}
	return true;
}
