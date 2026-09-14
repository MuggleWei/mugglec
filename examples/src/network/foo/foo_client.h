#ifndef FOO_CLIENT_H_
#define FOO_CLIENT_H_

#include "foo_handle.h"
#include "foo_session.h"

EXTERN_C_BEGIN

void foo_client_run(muggle_event_loop_t *evloop);

// ---------------- frame callbacks ----------------
void foo_client_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void foo_client_on_message(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void foo_client_on_close(muggle_event_loop_t *evloop,
						 muggle_socket_context_t *ctx);
void foo_client_on_release(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void foo_client_on_timer(muggle_event_loop_t *evloop);

// ---------------- message callbacks ----------------
void foo_client_on_ping(muggle_event_loop_t *evloop, foo_session_t *session,
						foo_msg_hdr_t *hdr, void *data);
void foo_client_on_rsp_login(muggle_event_loop_t *evloop,
							 foo_session_t *session, foo_msg_hdr_t *hdr,
							 void *data);

EXTERN_C_END

#endif // !FOO_CLIENT_H_
