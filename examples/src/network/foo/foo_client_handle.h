#ifndef FOO_CLIENT_HANDLE_H_
#define FOO_CLIENT_HANDLE_H_

#include "foo/foo_macro.h"
#include "foo_args.h"
#include "foo/foo_handle.h"

EXTERN_C_BEGIN

////////////// initialize handle //////////////
void init_tcp_client_handle(
	muggle_event_loop_t *evloop,
	muggle_socket_evloop_handle_t *handle,
	sys_args_t *args);

////////////// event loop callbacks //////////////
NET_FOO_EXPORT
void tcp_client_on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

NET_FOO_EXPORT
void tcp_client_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

NET_FOO_EXPORT
void tcp_client_on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

NET_FOO_EXPORT
void tcp_client_on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

NET_FOO_EXPORT
void tcp_client_on_timer(muggle_event_loop_t *evloop);

////////////// message callbacks //////////////

void tcp_client_on_rsp_login(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *msg);

void tcp_client_on_msg_ping(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *msg);

void tcp_server_on_rsp_sum(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *msg);

EXTERN_C_END

#endif /* ifndef FOO_CLIENT_HANDLE_H_ */
