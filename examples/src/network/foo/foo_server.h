#ifndef FOO_SERVER_H_
#define FOO_SERVER_H_

#include "foo_handle.h"
#include "foo_session.h"

EXTERN_C_BEGIN

void foo_server_run(muggle_event_loop_t *evloop);

// ---------------- frame callbacks ----------------
void foo_server_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void foo_server_on_connect(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void foo_server_on_message(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void foo_server_on_close(muggle_event_loop_t *evloop,
						 muggle_socket_context_t *ctx);
void foo_server_on_release(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void foo_server_on_timer(muggle_event_loop_t *evloop);

// ---------------- message callbacks ----------------
void foo_server_on_ping(muggle_event_loop_t *evloop, foo_session_t *session,
						foo_msg_hdr_t *hdr, void *data);
void foo_server_on_req_login(muggle_event_loop_t *evloop,
							 foo_session_t *session, foo_msg_hdr_t *hdr,
							 void *data);

EXTERN_C_END

#endif // !FOO_SERVER_H_
