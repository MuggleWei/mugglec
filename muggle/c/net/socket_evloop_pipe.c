#include "socket_evloop_pipe.h"
#include "muggle/c/base/sleep.h"
#include "muggle/c/log/log.h"
#include "muggle/c/os/sys.h"
#include <string.h>
#if MUGGLE_PLATFORM_WINDOWS
	#include "muggle/c/net/socket_utils.h"
#else
	#include <unistd.h>
	#include <fcntl.h>
#endif

enum {
	MUGGLE_SOCKET_EVLOOP_PIPE_READER = 0,
	MUGGLE_SOCKET_EVLOOP_PIPE_WRITER = 1,
};

int muggle_socket_evloop_pipe_init(muggle_socket_evloop_pipe_t *ev_pipe)
{
	memset(ev_pipe, 0, sizeof(*ev_pipe));

	muggle_spinlock_init(&ev_pipe->lock);

	ev_pipe->ctx[0].base.fd = MUGGLE_INVALID_SOCKET;
	ev_pipe->ctx[0].sock_type = MUGGLE_SOCKET_CTX_TYPE_PIPE;
	ev_pipe->ctx[1].base.fd = MUGGLE_INVALID_SOCKET;
	ev_pipe->ctx[1].sock_type = MUGGLE_SOCKET_CTX_TYPE_PIPE;

	muggle_socket_t fds[2];
#if MUGGLE_PLATFORM_WINDOWS
	if (muggle_socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
		return -1;
	}
#else
	if (pipe(fds) != 0) {
		return -1;
	}
#endif

	for (int i = 0; i < 2; i++) {
		muggle_socket_set_nonblock(fds[i], 1);

#if MUGGLE_PLATFORM_LINUX
		const int require_pipe_size = 32 * 1024 * sizeof(void *);
		int pipe_size = fcntl(fds[i], F_GETPIPE_SZ);
		if (pipe_size < require_pipe_size) {
			pipe_size = fcntl(fds[i], F_SETPIPE_SZ, require_pipe_size);
			if (pipe_size == -1) {
				MUGGLE_LOG_ERROR("failed set pipe size");
			}
		}
#elif MUGGLE_PLATFORM_WINDOWS
		const int require_socket_bufsize = 32 * 1024 * sizeof(void *);
		int bufsize = 0;
		socklen_t len_bufsize = sizeof(bufsize);
		int optname = SO_RCVBUF;
		if (i == MUGGLE_SOCKET_EVLOOP_PIPE_READER) {
			optname = SO_RCVBUF;
		} else if (i == MUGGLE_SOCKET_EVLOOP_PIPE_WRITER) {
			optname = SO_SNDBUF;
		}

		muggle_getsockopt(fds[i], SOL_SOCKET, optname, &bufsize, &len_bufsize);
		if (bufsize < require_socket_bufsize) {
			if (muggle_setsockopt(fds[i], SOL_SOCKET, optname, &bufsize,
								  sizeof(require_socket_bufsize)) != 0) {
				MUGGLE_LOG_ERROR("failed set socket %s buf size",
								 i == MUGGLE_SOCKET_EVLOOP_PIPE_READER ? "rcv" :
																		 "snd");
			}
		}
#endif
	}

	if (muggle_socket_ctx_init(&ev_pipe->ctx[0], fds[0], ev_pipe,
							   MUGGLE_SOCKET_CTX_TYPE_PIPE) != 0) {
		close(fds[0]);
		close(fds[1]);
		ev_pipe->ctx[0].base.fd = MUGGLE_INVALID_SOCKET;
		ev_pipe->ctx[1].base.fd = MUGGLE_INVALID_SOCKET;
		return -1;
	}

	if (muggle_socket_ctx_init(&ev_pipe->ctx[1], fds[1], ev_pipe,
							   MUGGLE_SOCKET_CTX_TYPE_PIPE) != 0) {
		close(fds[0]);
		close(fds[1]);
		ev_pipe->ctx[0].base.fd = MUGGLE_INVALID_SOCKET;
		ev_pipe->ctx[1].base.fd = MUGGLE_INVALID_SOCKET;
		return -1;
	}

	return 0;
}

void muggle_socket_evloop_pipe_destroy(muggle_socket_evloop_pipe_t *ev_pipe)
{
	if (ev_pipe->ctx[0].base.fd != MUGGLE_INVALID_SOCKET) {
		muggle_socket_ctx_close(&ev_pipe->ctx[0]);
	}

	if (ev_pipe->ctx[1].base.fd != MUGGLE_INVALID_SOCKET) {
		muggle_socket_ctx_close(&ev_pipe->ctx[1]);
	}
}

bool muggle_socket_evloop_pipe_write(muggle_socket_evloop_pipe_t *ev_pipe,
									 void *data)
{
	int n = 0;

	muggle_spinlock_lock(&ev_pipe->lock);

	muggle_atomic_thread_fence(muggle_memory_order_release);
	n = muggle_socket_block_write(
		ev_pipe->ctx[MUGGLE_SOCKET_EVLOOP_PIPE_WRITER].base.fd, &data,
		sizeof(void *), 400);

	muggle_spinlock_unlock(&ev_pipe->lock);

	return n == sizeof(void *) ? true : false;
}

void *muggle_socket_evloop_pipe_read(muggle_socket_evloop_pipe_t *ev_pipe)
{
	void *data = NULL;
	int offset = 0;
	int remain_bytes = (int)sizeof(void *);
	do {
		int n = muggle_socket_ctx_read(
			&ev_pipe->ctx[MUGGLE_SOCKET_EVLOOP_PIPE_READER],
			(char *)&data + offset, remain_bytes);
		if (n > 0) {
			offset += n;
			remain_bytes -= n;
			if (remain_bytes == 0) {
				break;
			}
		} else if (n == MUGGLE_EVENT_ERROR) {
			int last_errnum = muggle_sys_lasterror();
			if (last_errnum == MUGGLE_SYS_ERRNO_WOULDBLOCK ||
				last_errnum == MUGGLE_SYS_ERROR_AGAIN ||
				last_errnum == MUGGLE_SYS_ERRNO_INTR) {
				if (offset == 0) {
					return NULL;
				} else {
					muggle_nsleep(400);
				}
			} else {
				return NULL;
			}
		} else if (n == 0) {
			return NULL;
		}
	} while (1);
	muggle_atomic_thread_fence(muggle_memory_order_acquire);

	return data;
}

muggle_socket_context_t *
muggle_socket_evloop_pipe_get_writer(muggle_socket_evloop_pipe_t *ev_pipe)
{
	return &ev_pipe->ctx[MUGGLE_SOCKET_EVLOOP_PIPE_WRITER];
}

muggle_socket_context_t *
muggle_socket_evloop_pipe_get_reader(muggle_socket_evloop_pipe_t *ev_pipe)
{
	return &ev_pipe->ctx[MUGGLE_SOCKET_EVLOOP_PIPE_READER];
}
