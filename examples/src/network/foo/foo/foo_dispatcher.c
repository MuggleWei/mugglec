#include "foo_dispatcher.h"

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

void foo_dispatcher_append_codec(foo_dispatcher_t *dispatcher, foo_codec_t *codec)
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

bool foo_dispatcher_register(
	foo_dispatcher_t *dispatcher,
	uint32_t msg_type,
	foo_callback_func callback)
{
	if (msg_type >= FOO_MSG_TYPE_MAX)
	{
		LOG_ERROR("failed register callback, message type beyond max: msg_type=%lu",
			(unsigned long)msg_type);
		return false;
	}

	foo_callback_t *cb = dispatcher->callbacks + msg_type;
	if (cb->msg_type != 0)
	{
		LOG_ERROR("failed register callback, duplicated message type: msg_type=%lu",
			(unsigned long)msg_type);
		return false;
	}

	cb->msg_type = msg_type;
	cb->cb_func = callback;

	return true;
}

bool foo_dispatcher_handle(
	foo_dispatcher_t *dispatcher,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx)
{
	foo_codec_t *codec = dispatcher->codec_list_tail;
	return foo_dispatcher_decode(dispatcher, codec, evloop, ctx, NULL, 0);
}

bool foo_dispatcher_send(
	foo_dispatcher_t *dispatcher,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len)
{
	foo_codec_t *codec = dispatcher->codec_list_head;
	return foo_dispatcher_encode(dispatcher, codec, evloop, ctx, data, data_len);
}

bool foo_dispatcher_decode(
	foo_dispatcher_t *dispatcher,
	foo_codec_t *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len)
{
	if (codec)
	{
		return codec->decode(dispatcher, codec, evloop, ctx, data, data_len);
	}
	else
	{
		foo_msg_header_t *p_header = (foo_msg_header_t*)data;

		DEBUG_LOG_TRACE("dispatcher callback: msg_type=%lu", (unsigned long)p_header->msg_type);
		foo_callback_t *cb = &dispatcher->callbacks[p_header->msg_type];
		if (cb->cb_func)
		{
			cb->cb_func(evloop, ctx, data);
		}
		else
		{
			// return false;
			LOG_WARNING("failed find callback: msg_type=%lu", (unsigned long)p_header->msg_type);
			return true;
		}
	}

	return true;
}

bool foo_dispatcher_encode(
	foo_dispatcher_t *dispatcher,
	foo_codec_t *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len)
{
	if (codec)
	{
		return codec->encode(dispatcher, codec, evloop, ctx, data, data_len);
	}
	else
	{
		foo_msg_header_t *p_header = (foo_msg_header_t*)data;
		DEBUG_LOG_TRACE("dispatcher send: msg_type=%lu", htonl(p_header->msg_type));

		muggle_socket_ctx_write(ctx, data, data_len);
	}

	return true;
}
