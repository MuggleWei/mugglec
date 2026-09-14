#include "foo_session.h"
#include <assert.h>

void foo_session_init(foo_session_t *session)
{
	memset(session, 0, sizeof(*session));

	// bytes buffer manual init
	session->bytes_buf.buffer = session->bytes_buf_data;
	session->bytes_buf.c = sizeof(session->bytes_buf_data);
	session->bytes_buf.t = sizeof(session->bytes_buf_data);

	// initialize time counter
	muggle_time_counter_init(&session->read_idle_tc);
	muggle_time_counter_init(&session->write_idle_tc);
	muggle_time_counter_start(&session->read_idle_tc);
	muggle_time_counter_start(&session->write_idle_tc);

	// bind time wheel
	session->time_wheel_node.data = session;
}

void foo_session_destroy(foo_session_t *session)
{
	MUGGLE_UNUSED(session);
}

void foo_session_gen_addr_info(foo_session_t *session)
{
	muggle_event_fd fd = session->ctx.base.fd;
	muggle_socket_remote_ip_port(fd, session->remote_ip,
								 sizeof(session->remote_ip),
								 &session->remote_port);
	muggle_socket_local_ip_port(
		fd, session->local_ip, sizeof(session->local_ip), &session->local_port);
}

int foo_session_write(foo_session_t *session, foo_msg_hdr_t *hdr,
					  uint32_t datalen)
{
	// real write
	muggle_socket_context_t *ctx = (muggle_socket_context_t *)session;
	int n = muggle_socket_ctx_write(ctx, hdr, datalen);
	if (n != (int)datalen) {
		if (n == MUGGLE_EVENT_ERROR) {
			int last_errnum = muggle_sys_lasterror();
			LOG_ERROR("failed write message, "
					  "remote_ip: %s, remote_port: %d, errno: %d",
					  session->remote_ip, session->remote_port, last_errnum);
		} else {
			LOG_ERROR("failed write message, "
					  "remote_ip: %s, remote_port: %d, "
					  "expect_write: %u, actual_write: %d",
					  session->remote_ip, session->remote_port, datalen, n);
		}
		foo_session_shutdown(session);
	}
	return n;
}

#define RCV_UNIT_SIZE (256 * 1024)
void foo_session_read(foo_session_t *session, muggle_bytes_buffer_t *bytes_buf)
{
	muggle_socket_context_t *ctx = (muggle_socket_context_t *)session;

	// read bytes into bytes buffer
	while (1) {
		void *p = muggle_bytes_buffer_writer_fc(bytes_buf, RCV_UNIT_SIZE);
		if (p == NULL) {
			LOG_WARNING("bytes buffer full");
			muggle_ev_ctx_set_flag(&ctx->base, MUGGLE_EV_CTX_FLAG_CLOSED);
			break;
		}

		int n = muggle_socket_ctx_read(ctx, p, RCV_UNIT_SIZE);
		if (n > 0) {
			muggle_bytes_buffer_writer_move_n(bytes_buf, p, n);
		} else {
			break;
		}
	}
}

void foo_session_shutdown(foo_session_t *session)
{
	muggle_socket_ctx_shutdown((muggle_socket_context_t *)session);
}

static void assign_end_tick(const muggle_time_counter_t *tc,
							muggle_time_counter_t *dst)
{
#if MUGGLE_PLATFORM_WINDOWS
	static_assert(sizeof(dst->end) == sizeof(tc->start), "");
	memcpy(&dst->end, &tc->start, sizeof(dst->end));
#else
	static_assert(sizeof(dst->end_ts) == sizeof(tc->start_ts), "");
	memcpy(&dst->end_ts, &tc->start_ts, sizeof(dst->end_ts));
#endif
}

bool foo_session_is_read_idle(foo_session_t *session, muggle_time_counter_t *tc,
							  int64_t timeout_ms)
{
	assign_end_tick(tc, &session->read_idle_tc);
	int64_t elapsed_ms =
		muggle_time_counter_interval_ms(&session->read_idle_tc);
	return elapsed_ms >= timeout_ms;
}
bool foo_session_is_write_idle(foo_session_t *session,
							   muggle_time_counter_t *tc, int64_t timeout_ms)
{
	assign_end_tick(tc, &session->write_idle_tc);
	int64_t elapsed_ms =
		muggle_time_counter_interval_ms(&session->write_idle_tc);
	return elapsed_ms >= timeout_ms;
}
