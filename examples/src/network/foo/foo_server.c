#include "foo_server.h"
#include "foo_msg.h"
#include <inttypes.h>

static void server_listen(muggle_event_loop_t *evloop, foo_handle_t *foo_handle)
{
	foo_config_t *cfg = foo_handle->cfg;

	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = muggle_tcp_listen_with_cb(cfg->host, cfg->port, 512,
									   foo_set_socket_opt_before_conn, NULL);
		if (fd == MUGGLE_INVALID_SOCKET) {
			LOG_ERROR("failed tcp listen %s:%s", cfg->host, cfg->port);
			muggle_msleep(3000);
		}
	} while (fd == MUGGLE_INVALID_SOCKET);

	foo_session_t *session =
		(foo_session_t *)foo_handle_alloc_session(foo_handle);
	foo_session_init(session);
	muggle_socket_ctx_init((muggle_socket_context_t *)session, fd, NULL,
						   MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN);
	muggle_socket_evloop_add_ctx(evloop, (muggle_socket_context_t *)session);
}

void foo_server_run(muggle_event_loop_t *evloop)
{
	foo_handle_t *foo_handle = (foo_handle_t *)muggle_evloop_get_data(evloop);
	if (foo_handle == NULL) {
		LOG_ERROR("failed get evloop handle");
		return;
	}
	foo_config_t *cfg = foo_handle->cfg;

	// register callbacks
	foo_dispatcher_t *dispatcher = &foo_handle->dispatcher;
	foo_dispatcher_register(dispatcher, FOO_MSG_ID_PING, foo_server_on_ping);
	foo_dispatcher_register(dispatcher, FOO_MSG_ID_REQ_LOGIN,
							foo_server_on_req_login);

	// initialize socket handle
	muggle_socket_evloop_handle_t *handle = &foo_handle->handle;
	muggle_socket_evloop_handle_init(handle);
	muggle_socket_evloop_handle_set_cb_add_ctx(handle, foo_server_on_add_ctx);
	muggle_socket_evloop_handle_set_cb_conn(handle, foo_server_on_connect);
	muggle_socket_evloop_handle_set_cb_msg(handle, foo_server_on_message);
	muggle_socket_evloop_handle_set_cb_close(handle, foo_server_on_close);
	muggle_socket_evloop_handle_set_cb_release(handle, foo_server_on_release);
	muggle_socket_evloop_handle_set_cb_timer(handle, foo_server_on_timer);
	muggle_socket_evloop_handle_set_timer_interval(
		handle, cfg->evloop_timer_interval_ms);
	muggle_socket_evloop_handle_set_alloc_free(handle, foo_handle,
											   foo_handle_alloc_session,
											   foo_handle_recycle_session);
	muggle_socket_evloop_handle_attach(handle, evloop);

	// listen
	server_listen(evloop, foo_handle);

	// run
	muggle_evloop_run(evloop);

	// cleanup
	foo_handle_destroy(foo_handle);
}

// ---------------- frame callbacks ----------------
void foo_server_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	switch (ctx->sock_type) {
	case MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN: {
		foo_session_t *session = (foo_session_t *)ctx;

		foo_session_gen_addr_info(session);

		LOG_INFO("listen session add into evloop, local_ip: %s, local_port: %d",
				 session->local_ip, session->local_port);
	} break;
	default: {
		LOG_ERROR("socket(type=%d) add into evloop", ctx->sock_type);
	} break;
	}
}
void foo_server_on_connect(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	foo_handle_t *handle = muggle_evloop_get_data(evloop);
	foo_session_t *session = (foo_session_t *)ctx;

	foo_set_socket_opt_after_conn(ctx->base.fd, NULL);

	foo_session_gen_addr_info(session);
	muggle_time_wheel_insert(&handle->time_wheel, &session->time_wheel_node);

	LOG_INFO("client session connect, remote_ip: %s, remote_port: %d",
			 session->remote_ip, session->remote_port);
}
void foo_server_on_message(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	foo_handle_t *handle = (foo_handle_t *)muggle_evloop_get_data(evloop);
	foo_handle_on_message(handle, (foo_session_t *)ctx);
}
void foo_server_on_close(muggle_event_loop_t *evloop,
						 muggle_socket_context_t *ctx)
{
	foo_handle_t *handle = (foo_handle_t *)muggle_evloop_get_data(evloop);
	foo_session_t *session = (foo_session_t *)ctx;

	switch (ctx->sock_type) {
	case MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN: {
		LOG_ERROR("listen session closed, local_ip: %s, local_port: %d",
				  session->local_ip, session->local_port);
	} break;
	case MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT: {
		LOG_INFO("client session closed, remote_ip: %s, remote_port: %d",
				 session->remote_ip, session->remote_port);

		muggle_time_wheel_remove(&handle->time_wheel,
								 &session->time_wheel_node);
	} break;
	default: {
		LOG_ERROR("unknown session(type=%d) closed", ctx->sock_type);
	} break;
	}
}
void foo_server_on_release(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	foo_session_t *session = (foo_session_t *)ctx;

	switch (ctx->sock_type) {
	case MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN: {
		LOG_ERROR("listen session release, local_ip: %s, local_port: %d",
				  session->local_ip, session->local_port);
	} break;
	case MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT: {
		LOG_INFO("client session release, remote_ip: %s, remote_port: %d",
				 session->remote_ip, session->remote_port);
	} break;
	default: {
		LOG_ERROR("unknown session(type=%d) release", ctx->sock_type);
	} break;
	}
}
void foo_server_on_timer(muggle_event_loop_t *evloop)
{
	// calculate timer
	foo_handle_t *handle = (foo_handle_t *)muggle_evloop_get_data(evloop);
	muggle_time_counter_end(&handle->timer_tc);
	int64_t elapsed_ms = muggle_time_counter_interval_ms(&handle->timer_tc);
	if (elapsed_ms < handle->timer_interval_ms) {
		return;
	}
	muggle_time_counter_move_end_to_start(&handle->timer_tc);

	// update time wheel
	muggle_time_wheel_update(&handle->time_wheel, foo_time_wheel_update,
							 handle);
}

// ---------------- message callbacks ----------------
void foo_server_on_ping(muggle_event_loop_t *evloop, foo_session_t *session,
						foo_msg_hdr_t *hdr, void *data)
{
	MUGGLE_UNUSED(evloop);
	MUGGLE_UNUSED(hdr);
	MUGGLE_UNUSED(data);

	LOG_DEBUG("rcv ping, remote_ip: %s, remote_port: %d", session->remote_ip,
			  session->remote_port);
}
void foo_server_on_req_login(muggle_event_loop_t *evloop,
							 foo_session_t *session, foo_msg_hdr_t *hdr,
							 void *data)
{
	MUGGLE_UNUSED(evloop);

	if (hdr->payload_len != sizeof(foo_msg_req_login_t)) {
		LOG_ERROR("invalid message payload length, msg_id=%u", hdr->msg_id);
		foo_session_shutdown(session);
		return;
	}

	foo_msg_req_login_t *req = (foo_msg_req_login_t *)data;
	LOG_INFO("rcv req login message, "
			 "remote_ip: %s, remote_port: %d, user_id=%" PRIi32
			 ", password=******",
			 session->remote_ip, session->remote_port, req->user_id);

	// do something check user password
	LOG_INFO("success user login, "
			 "remote_ip: %s, remote_port: %d, user_id=%" PRIi32,
			 session->remote_ip, session->remote_port, req->user_id);

	FOO_NEW_STACK_MSG(FOO_MSG_ID_RSP_LOGIN, foo_msg_rsp_login_t, rsp);
	rsp->user_id = req->user_id;
	rsp->err.err_id = 0;
	FOO_SEND_MSG(session, rsp);

	LOG_INFO("snd rsp login message");
}
