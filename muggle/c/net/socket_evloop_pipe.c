#include "socket_evloop_pipe.h"
#include "muggle/c/log/log.h"
#include <string.h>
#if MUGGLE_PLATFORM_WINDOWS
	#include "muggle/c/net/socket_utils.h"
#else
	#include <unistd.h>
	#include <fcntl.h>
#endif

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
		int pipe_size = fcntl(fds[i], F_SETPIPE_SZ, 16 * 1024 * sizeof(void*));
		if (pipe_size == -1) {
			MUGGLE_LOG_ERROR("failed set pipe size");
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
	n = muggle_socket_ctx_write(&ev_pipe->ctx[MUGGLE_SOCKET_EVLOOP_PIPE_WRITER],
								&data, sizeof(void *));

	muggle_spinlock_unlock(&ev_pipe->lock);

	return n == sizeof(void *) ? true : false;
}

void *muggle_socket_evloop_pipe_read(muggle_socket_evloop_pipe_t *ev_pipe)
{
	void *data = NULL;
	muggle_socket_ctx_read(&ev_pipe->ctx[MUGGLE_SOCKET_EVLOOP_PIPE_READER],
						   &data, sizeof(void *));
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
