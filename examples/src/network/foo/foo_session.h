#ifndef FOO_SESSION_H_
#define FOO_SESSION_H_

#include "foo_config.h"
#include "foo_msg.h"

EXTERN_C_BEGIN

#define FOO_SESSION_BYTES_BUFFER_CAP (2 * 1024 * 1024)
#define FOO_SESSION_RCVBUF_SIZE (1 * 1024 * 1024)
#define FOO_SESSION_SNDBUF_SIZE (1 * 1024 * 1024)

typedef struct {
	muggle_socket_context_t ctx;
	muggle_bytes_buffer_t bytes_buf;
	char bytes_buf_data[FOO_SESSION_BYTES_BUFFER_CAP];
	char remote_ip[MUGGLE_SOCKET_ADDR_STRLEN];
	int remote_port;
	char local_ip[MUGGLE_SOCKET_ADDR_STRLEN];
	int local_port;
	muggle_time_counter_t read_idle_tc;
	muggle_time_counter_t write_idle_tc;
	muggle_time_wheel_node_t time_wheel_node;
} foo_session_t;

void foo_session_init(foo_session_t *session);

void foo_session_destroy(foo_session_t *session);

void foo_session_gen_addr_info(foo_session_t *session);

int foo_session_write(foo_session_t *session, foo_msg_hdr_t *hdr,
					  uint32_t datalen);

void foo_session_read(foo_session_t *session, muggle_bytes_buffer_t *bytes_buf);

void foo_session_shutdown(foo_session_t *session);

bool foo_session_is_read_idle(foo_session_t *session, muggle_time_counter_t *tc,
							  int64_t timeout_ms);
bool foo_session_is_write_idle(foo_session_t *session,
							   muggle_time_counter_t *tc, int64_t timeout_ms);

EXTERN_C_END

#endif // !FOO_SESSION_H_
