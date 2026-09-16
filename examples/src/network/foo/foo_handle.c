#include "foo_handle.h"
#include "foo_session.h"
#include <inttypes.h>

bool foo_handle_init(foo_handle_t *handle, muggle_event_loop_t *evloop,
					 foo_config_t *cfg)
{
	memset(handle, 0, sizeof(*handle));
	handle->cfg = cfg;
	handle->evloop = evloop;

	muggle_time_counter_init(&handle->timer_tc);
	muggle_time_counter_start(&handle->timer_tc);

	handle->timer_interval_ms = cfg->logic_timer_interval_ms;
	handle->read_idle_ms = cfg->read_idle_ms;
	handle->write_idle_ms = cfg->write_idle_ms;

	if (!muggle_time_wheel_init(&handle->time_wheel, cfg->n_time_wheel_slot,
								handle->timer_interval_ms,
								MUGGLE_TIME_WHEEL_INSERT_MODE_LOOP)) {
		LOG_ERROR("failed init time wheel");
		return false;
	}

	if (!muggle_memory_pool_init_thp(&handle->session_pool, 128,
									 sizeof(foo_session_t))) {
		LOG_ERROR("failed initialize session pool");
		return false;
	}

	foo_dispatcher_init(&handle->dispatcher);

	muggle_evloop_set_data(evloop, handle);

	return true;
}

void foo_handle_destroy(foo_handle_t *handle)
{
	muggle_memory_pool_destroy(&handle->session_pool);
	muggle_time_wheel_destroy(&handle->time_wheel);
}

muggle_socket_context_t *foo_handle_alloc_session(void *p)
{
	foo_handle_t *handle = (foo_handle_t *)p;
	muggle_memory_pool_t *pool = (muggle_memory_pool_t *)&handle->session_pool;
	foo_session_t *session = (foo_session_t *)muggle_memory_pool_alloc(pool);
	foo_session_init(session);
	return (muggle_socket_context_t *)session;
}
void foo_handle_recycle_session(void *p, muggle_socket_context_t *data)
{
	foo_handle_t *handle = (foo_handle_t *)p;
	muggle_memory_pool_t *pool = (muggle_memory_pool_t *)&handle->session_pool;
	muggle_memory_pool_free(pool, data);
}

void foo_set_socket_opt_before_conn(muggle_socket_t fd, void *p)
{
	MUGGLE_UNUSED(p);

	int require_rcv_bufsize = 1 * 1024 * 1024;
	int require_snd_bufsize = 1 * 1024 * 1024;

	int default_rcv_bufsize = 0;
	muggle_socklen_t len_rcv_bufsize = sizeof(default_rcv_bufsize);
	muggle_getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &default_rcv_bufsize,
					  &len_rcv_bufsize);
	if (default_rcv_bufsize < require_rcv_bufsize) {
		if (muggle_setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &require_rcv_bufsize,
							  sizeof(require_rcv_bufsize)) != 0) {
			LOG_ERROR("failed set socket rcv buf size");
		}
	}

	int default_snd_bufsize = 0;
	muggle_socklen_t len_snd_bufsize = sizeof(default_snd_bufsize);
	muggle_getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &default_snd_bufsize,
					  &len_snd_bufsize);
	if (default_snd_bufsize < require_snd_bufsize) {
		if (muggle_setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &require_snd_bufsize,
							  sizeof(require_snd_bufsize)) != 0) {
			LOG_ERROR("failed set socket snd buf size");
		}
	}
}

void foo_set_socket_opt_after_conn(muggle_socket_t fd, void *p)
{
	MUGGLE_UNUSED(p);

	int enable = 1;
	if (muggle_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable,
						  sizeof(enable)) != 0) {
		LOG_ERROR("failed set TCP NODELAY");
	}
}

void foo_handle_on_message(foo_handle_t *handle, foo_session_t *session)
{
	// read bytes into bytes buffer
	muggle_bytes_buffer_t *bytes_buf = &session->bytes_buf;
	foo_session_read(session, bytes_buf);

	// decode message and dispatch
	foo_handle_msg_decode_dispatch(handle, session, bytes_buf);
}

#define ADLER_MOD 65521u // argest prime number less than 2^16
uint32_t cal_checksum_adler32(const uint8_t *data, size_t len)
{
	uint32_t a = 1;
	uint32_t b = 0;
	for (size_t i = 0; i < len; ++i) {
		a += data[i];
		b += a;

		a %= ADLER_MOD;
		b %= ADLER_MOD;
	}
	return (b << 16) | a;
}
bool check_checksum(foo_msg_hdr_t *hdr)
{
	uint32_t expect_checksum = cal_checksum_adler32(
		(uint8_t *)hdr, sizeof(foo_msg_hdr_t) + hdr->payload_len);
	foo_msg_tail_t *tail =
		(foo_msg_tail_t *)((char *)(hdr + 1) + hdr->payload_len);
	return expect_checksum == tail->checksum;
}

void foo_handle_msg_decode_dispatch(foo_handle_t *handle,
									foo_session_t *session,
									muggle_bytes_buffer_t *bytes_buf)
{
	// update read idle tc
	muggle_time_counter_start(&session->read_idle_tc);

	// decode message
	muggle_event_loop_t *evloop = handle->evloop;
	foo_dispatcher_t *dispatcher = &handle->dispatcher;
	foo_msg_hdr_t msg_hdr;
	while (1) {
		if (!muggle_bytes_buffer_fetch(bytes_buf, (int)sizeof(msg_hdr),
									   &msg_hdr)) {
			// readable bytes less than sizeof message header
			break;
		}

		// check message length
		uint32_t total_bytes =
			(uint32_t)(sizeof(foo_msg_hdr_t) + msg_hdr.payload_len +
					   sizeof(foo_msg_tail_t));
		int readable = muggle_bytes_buffer_readable(bytes_buf);
		if ((uint32_t)readable < total_bytes) {
			break;
		}

		// check magic word
		if (memcmp(&msg_hdr.magic, FOO_MSG_HDR_MAGIC_WORD, 4) != 0) {
			LOG_ERROR("failed check message magic word");
			foo_session_shutdown(session);
			break;
		}

		// handle message
		foo_msg_hdr_t *p_hdr = (foo_msg_hdr_t *)muggle_bytes_buffer_reader_fc(
			bytes_buf, (uint32_t)total_bytes);
		if (p_hdr) {
			// continuous memory
			if (!check_checksum(p_hdr)) {
				LOG_ERROR("failed check message checksum");
				foo_session_shutdown(session);
				break;
			}

			// dispatch
			foo_dispatcher_dispatch(dispatcher, evloop, session, p_hdr);

			muggle_bytes_buffer_reader_move(bytes_buf, (int)total_bytes);
		} else {
			// discontinuous memory
			void *buf = malloc(total_bytes);
			muggle_bytes_buffer_read(bytes_buf, (int)total_bytes, buf);

			// check checksum
			if (!check_checksum((foo_msg_hdr_t *)buf)) {
				LOG_ERROR("failed checksum");
				foo_session_shutdown(session);
				break;
			}

			// dispatch
			foo_dispatcher_dispatch(dispatcher, evloop, session,
									(foo_msg_hdr_t *)buf);

			free(buf);
		}
	}
}

int foo_handle_msg_encode_send(foo_session_t *session, foo_msg_hdr_t *hdr,
							   uint32_t datalen)
{
	// fillup hdr
	memcpy(hdr->magic, FOO_MSG_HDR_MAGIC_WORD, 4);
	hdr->flags[FOO_MSG_HDR_FLAG_VERSION] = FOO_PROTOCOL_VERSION;

	// fillup tail
	foo_msg_tail_t *tail =
		(foo_msg_tail_t *)((char *)(hdr + 1) + hdr->payload_len);
	tail->checksum = cal_checksum_adler32(
		(uint8_t *)hdr, sizeof(foo_msg_hdr_t) + hdr->payload_len);

	// update write idle tc
	muggle_time_counter_start(&session->write_idle_tc);

	// send
	return foo_session_write(session, hdr, datalen);
}

void foo_time_wheel_update(muggle_time_wheel_node_t *node, void *user_data)
{
	foo_session_t *session = (foo_session_t *)node->data;
	foo_handle_t *handle = (foo_handle_t *)user_data;
	muggle_time_counter_t *tc = &handle->timer_tc;
	if (foo_session_is_write_idle(session, tc, handle->write_idle_ms)) {
		foo_on_write_idle(session);
	}
	if (foo_session_is_read_idle(session, tc, handle->read_idle_ms)) {
		foo_on_read_idle(session);
	}
}

void foo_on_write_idle(foo_session_t *session)
{
	int64_t elapsed = muggle_time_counter_interval_ms(&session->write_idle_tc);
	LOG_DEBUG("on write idle, "
			  "remote_ip: %s, remote_port: %d, elapsed: %" PRIi64,
			  session->remote_ip, session->remote_port, elapsed);

	FOO_NEW_STACK_MSG(FOO_MSG_ID_PING, foo_msg_ping_t, msg);
	msg->sec = 0;
	msg->nsec = 0;
	FOO_SEND_MSG(session, msg);

	LOG_DEBUG("snd ping, remote_ip: %s, remote_port: %d", session->remote_ip,
			  session->remote_port);
}

void foo_on_read_idle(foo_session_t *session)
{
	int64_t elapsed = muggle_time_counter_interval_ms(&session->read_idle_tc);
	LOG_INFO("on read idle, "
			 "remote_ip: %s, remote_port: %d, elapsed: %" PRIi64,
			 session->remote_ip, session->remote_port, elapsed);
	foo_session_shutdown(session);
}
