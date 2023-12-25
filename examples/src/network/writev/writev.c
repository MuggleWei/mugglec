#include "muggle/c/muggle_c.h"
#include <sys/socket.h>

#if MUGGLE_PLATFORM_LINUX
#else
typedef struct listen_thread_args {
	muggle_socket_t *fd;
	muggle_socket_t listen_fd;
} listen_thread_args_t;

static muggle_thread_ret_t tcp_listen_routine(void *p_args)
{
	listen_thread_args_t *listen_args = (listen_thread_args_t *)p_args;

	muggle_socket_t fd = accept(listen_args->listen_fd, NULL, NULL);
	if (fd == MUGGLE_INVALID_SOCKET) {
		LOG_ERROR("failed accept");
		goto listen_thread_exit;
	}

	*listen_args->fd = fd;

listen_thread_exit:
	muggle_socket_close(listen_args->listen_fd);

	return 0;
}

static bool create_tcp_socket_pair(muggle_socket_t fds[2])
{
	const char *host = "127.0.0.1";
	const char *port = "10102";

	fds[0] = MUGGLE_INVALID_SOCKET;
	fds[1] = MUGGLE_INVALID_SOCKET;

	muggle_socket_t listen_fd = muggle_tcp_listen(host, port, 8);
	if (listen_fd == MUGGLE_INVALID_SOCKET) {
		char errmsg[265];
		muggle_socket_strerror(muggle_socket_lasterror(), errmsg,
							   sizeof(errmsg));
		LOG_ERROR("failed listen %s:%s, errmsg: %s", host, port, errmsg);
		return false;
	}

	listen_thread_args_t listen_args;
	listen_args.listen_fd = listen_fd;
	listen_args.fd = &fds[0];
	muggle_thread_t th_listen;
	muggle_thread_create(&th_listen, tcp_listen_routine, &listen_args);

	fds[1] = muggle_tcp_connect(host, port, 3);

	muggle_thread_join(&th_listen);

	if (fds[0] == MUGGLE_INVALID_SOCKET || fds[1] == MUGGLE_INVALID_SOCKET) {
		if (fds[0] != MUGGLE_INVALID_SOCKET) {
			muggle_socket_close(fds[0]);
		}
		if (fds[1] != MUGGLE_INVALID_SOCKET) {
			muggle_socket_close(fds[1]);
		}
		return false;
	}

	return true;
}
#endif

typedef struct msg_hdr {
	uint32_t msg_id;
	uint32_t payload_len;
} msg_hdr_t;

typedef struct msg_data {
	uint32_t u32;
	int32_t i32;
	char s[16];
} msg_data_t;

typedef struct msg_tail {
	union {
		uint32_t checksum;
		struct {
			uint32_t checksum_fletcher16;
		};
	};
} msg_tail_t;

uint16_t gen_checksum_fletcher16(muggle_socket_iovec_t *iov, int iovcnt)
{
	uint16_t sum1 = 0;
	uint16_t sum2 = 0;

	for (int i = 0; i < iovcnt; i++) {
		muggle_socket_iovec_t *p_iov = iov + i;
		uint8_t *data = (uint8_t *)MUGGLE_SOCKET_IOVEC_GET_BUF(p_iov);
		for (size_t idx = 0; idx < MUGGLE_SOCKET_IOVEC_GET_LEN(p_iov); idx++) {
			sum1 = (sum1 + data[idx]) & 0xff;
			sum2 = (sum2 + sum1) & 0xff;
		}
	}

	return (sum2 << 8) | sum1;
}

void run_writev(muggle_socket_t fd)
{
	msg_hdr_t hdr;
	msg_data_t data;
	msg_tail_t tail;

	memset(&hdr, 0, sizeof(hdr));
	memset(&data, 0, sizeof(data));
	memset(&tail, 0, sizeof(tail));

	hdr.msg_id = 1;
	hdr.payload_len = sizeof(data);

	data.u32 = 5;
	data.i32 = 6;
	strncpy(data.s, "hello world", sizeof(data.s) - 1);

	muggle_socket_iovec_t iov[3];
	MUGGLE_SOCKET_IOVEC_SET_BUF(&iov[0], &hdr);
	MUGGLE_SOCKET_IOVEC_SET_LEN(&iov[0], sizeof(hdr));
	MUGGLE_SOCKET_IOVEC_SET_BUF(&iov[1], &data);
	MUGGLE_SOCKET_IOVEC_SET_LEN(&iov[1], sizeof(data));
	MUGGLE_SOCKET_IOVEC_SET_BUF(&iov[2], &tail);
	MUGGLE_SOCKET_IOVEC_SET_LEN(&iov[2], sizeof(tail));
	tail.checksum_fletcher16 = gen_checksum_fletcher16(&iov[0], 2);

	LOG_INFO("write: \n"
			 "hdr | msg_id=%u, payload_len=%u\n"
			 "data| u32=%u, i32=%d, s=%s\n"
			 "tail| checksum=%u",
			 hdr.msg_id, hdr.payload_len, data.u32, data.i32, data.s,
			 (uint32_t)tail.checksum_fletcher16);

	muggle_socket_writev(fd, &iov[0], sizeof(iov) / sizeof(iov[0]));
}

void run_read(muggle_socket_t fd)
{
	char buf[256];
	int num_bytes = muggle_socket_read(fd, buf, sizeof(buf));
	if (num_bytes !=
		sizeof(msg_hdr_t) + sizeof(msg_data_t) + sizeof(msg_tail_t)) {
		LOG_ERROR("failed read message");
		return;
	}

	msg_hdr_t *hdr = (msg_hdr_t *)buf;
	msg_data_t *data = (msg_data_t *)(hdr + 1);
	msg_tail_t *tail = (msg_tail_t *)(data + 1);

	LOG_INFO("read: \n"
			 "hdr | msg_id=%u, payload_len=%u\n"
			 "data| u32=%u, i32=%d, s=%s\n"
			 "tail| checksum=%u",
			 hdr->msg_id, hdr->payload_len, data->u32, data->i32, data->s,
			 (uint32_t)tail->checksum_fletcher16);
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_DEBUG, -1, NULL);

	muggle_socket_t fds[2];
#if MUGGLE_PLATFORM_LINUX
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
		LOG_ERROR("failed create socket pair");
		exit(EXIT_FAILURE);
	}
#else
	if (!create_tcp_socket_pair(fds)) {
		LOG_ERROR("failed create socket pair");
		exit(EXIT_FAILURE);
	}
#endif
	muggle_socket_set_nonblock(fds[0], 1);
	muggle_socket_set_nonblock(fds[1], 1);

	run_writev(fds[0]);
	run_read(fds[1]);

	muggle_socket_close(fds[0]);
	muggle_socket_close(fds[1]);

	return 0;
}
