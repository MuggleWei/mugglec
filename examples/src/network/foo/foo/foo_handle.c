#include "foo_handle.h"
#include "foo/foo_dispatcher.h"
#include "foo/foo_codec.h"
#include "foo/codec/codec_bytes.h"
#include "foo/codec/codec_endian.h"

void foo_handle_init(muggle_event_loop_t *evloop, void *user_data)
{
	foo_evloop_data_t *evloop_data =
		(foo_evloop_data_t*)malloc(sizeof(foo_evloop_data_t));
	memset(evloop_data, 0, sizeof(*evloop_data));
	evloop_data->recv_unit_size = 4 * 1024;
	evloop_data->msg_len_limit = 65536;
	evloop_data->user_data = user_data;

	// init connection list
	muggle_linked_list_init(&evloop_data->conn_list, 0);

	// init dispatcher
	foo_dispatcher_init(&evloop_data->dispatcher);

	foo_codec_endian_t *endian_codec = (foo_codec_endian_t*)malloc(sizeof(foo_codec_endian_t));
	foo_codec_endian_init(endian_codec);
	foo_dispatcher_append_codec(&evloop_data->dispatcher, (foo_codec_t*)endian_codec);

	foo_codec_bytes_t *bytes_codec = (foo_codec_bytes_t*)malloc(sizeof(foo_codec_bytes_t));
	foo_codec_bytes_init(bytes_codec);
	foo_dispatcher_append_codec(&evloop_data->dispatcher, (foo_codec_t*)bytes_codec);

	// set event loop data
	muggle_evloop_set_data(evloop, evloop_data);
}

void foo_handle_destroy(muggle_event_loop_t *evloop)
{
	foo_evloop_data_t *evloop_data = muggle_evloop_get_data(evloop);

	foo_codec_t *codec = evloop_data->dispatcher.codec_list_head;
	while (codec)
	{
		foo_codec_t *next_codec = codec->next;
		free(codec);
		codec = next_codec;
	}

	muggle_linked_list_destroy(&evloop_data->conn_list, NULL, NULL);

	free(evloop_data);
}

foo_dispatcher_t* foo_handle_dipatcher(muggle_event_loop_t *evloop)
{
	foo_evloop_data_t *evloop_data = (foo_evloop_data_t*)muggle_evloop_get_data(evloop);
	return &evloop_data->dispatcher;
}

muggle_linked_list_t* foo_handle_conn_list(muggle_event_loop_t *evloop)
{
	foo_evloop_data_t *evloop_data = (foo_evloop_data_t*)muggle_evloop_get_data(evloop);
	return &evloop_data->conn_list;
}

void foo_on_connect(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	// init socket context data
	foo_socket_ctx_data_t *ctx_data = (foo_socket_ctx_data_t*)malloc(sizeof(foo_socket_ctx_data_t));
	memset(ctx_data, 0, sizeof(*ctx_data));

	ctx_data->user_id = 0;
	muggle_socket_remote_addr(muggle_socket_ctx_get_fd(ctx), ctx_data->straddr, sizeof(ctx_data->straddr), 0);
	muggle_bytes_buffer_init(&ctx_data->bytes_buf, 4 * 1024 * 1024);
	ctx_data->last_sec = (uint64_t)time(NULL);

	// set socket context data
	muggle_socket_ctx_set_data(ctx, ctx_data);

	// append to connection list
	muggle_linked_list_t *conn_list = foo_handle_conn_list(evloop);
	muggle_linked_list_node_t *node = muggle_linked_list_append(conn_list, NULL, ctx);
	ctx_data->conn_node = node;
}

void foo_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
	if (!foo_dispatcher_handle(dispatcher, evloop, ctx))
	{
		muggle_socket_ctx_shutdown(ctx);
	}
}

void foo_on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	foo_socket_ctx_data_t *ctx_data = (foo_socket_ctx_data_t*)muggle_socket_ctx_get_data(ctx);
	if (ctx_data == NULL)
	{
		return;
	}

	// remove from connection list
	muggle_linked_list_t *conn_list = foo_handle_conn_list(evloop);
	muggle_linked_list_remove(conn_list, ctx_data->conn_node, NULL, NULL);

	// destroy resources
	muggle_bytes_buffer_destroy(&ctx_data->bytes_buf);
	free(ctx_data);
}
