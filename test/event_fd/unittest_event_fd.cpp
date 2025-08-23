#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

#define BUFSIZE_16K (8 * 1024)

class TestEventFdFixture : public ::testing::Test {
public:
	virtual void SetUp() override
	{
#if MUGGLE_PLATFORM_WINDOWS
		int ret = muggle_socketpair(AF_INET, SOCK_STREAM, 0, fds);
#else
		int ret = muggle_socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
#endif
		ASSERT_EQ(ret, 0);

		int default_rcv_bufsize = 0;
		int default_snd_bufsize = 0;
		int rcv_bufsize = BUFSIZE_16K;
		int snd_bufsize = BUFSIZE_16K;
		muggle_socklen_t len = sizeof(int);
		for (size_t i = 0; i < sizeof(fds) / sizeof(fds[0]); ++i) {
			// set rcv bufsize
			ret = muggle_getsockopt(fds[i], SOL_SOCKET, SO_RCVBUF,
									&default_rcv_bufsize, &len);
			ASSERT_EQ(ret, 0);
			if (default_rcv_bufsize != rcv_bufsize) {
				ret = muggle_setsockopt(fds[i], SOL_SOCKET, SO_RCVBUF,
										&rcv_bufsize, sizeof(rcv_bufsize));
				ASSERT_EQ(ret, 0);
			}

			// set snd bufsize
			ret = muggle_getsockopt(fds[i], SOL_SOCKET, SO_SNDBUF,
									&default_snd_bufsize, &len);
			ASSERT_EQ(ret, 0);
			if (default_snd_bufsize != snd_bufsize) {
				ret = muggle_setsockopt(fds[i], SOL_SOCKET, SO_SNDBUF,
										&snd_bufsize, sizeof(snd_bufsize));
				ASSERT_EQ(ret, 0);
			}

			// set non-blocking
			ret = muggle_ev_fd_set_nonblock(fds[i], 1);
			ASSERT_EQ(ret, 0);
		}
	}
	virtual void TearDown() override
	{
		if (fds[0] != MUGGLE_INVALID_SOCKET) {
			muggle_socket_close(fds[0]);
			fds[0] = MUGGLE_INVALID_SOCKET;
		}
		if (fds[1] != MUGGLE_INVALID_SOCKET) {
			muggle_socket_close(fds[1]);
			fds[1] = MUGGLE_INVALID_SOCKET;
		}
	}

protected:
	muggle_event_fd fds[2];
};

TEST_F(TestEventFdFixture, read_empty)
{
	char buf[1024];
	int ret = muggle_ev_fd_read(fds[0], buf, sizeof(buf));
	ASSERT_EQ(ret, -1);
	ASSERT_EQ(MUGGLE_EVENT_LAST_ERRNO, MUGGLE_SYS_ERRNO_WOULDBLOCK);
}

TEST_F(TestEventFdFixture, write_read)
{
	char buf[1024];
	int ret = 0;
	int *p = (int *)buf;
	*p = 5;

	ret = muggle_ev_fd_write(fds[0], buf, sizeof(buf));
	ASSERT_EQ(ret, (int)sizeof(buf));

	memset(buf, 0, sizeof(buf));
	ret = muggle_ev_fd_read(fds[1], buf, sizeof(buf));
	ASSERT_EQ(ret, (int)sizeof(buf));

	ASSERT_EQ(5, *(int *)buf);
}

TEST_F(TestEventFdFixture, write_would_block)
{
	char buf[1005];
	int ret = 0;
	int w_val = 0;
	int r_val = 0;
	int32_t w_n_bytes = 0;
	int32_t r_n_bytes = 0;
	int *p = (int *)buf;
	while (1) {
		*p = ++w_val;
		ret = muggle_ev_fd_write(fds[0], buf, sizeof(buf));
		if (ret != (int)sizeof(buf)) {
			ASSERT_EQ(MUGGLE_EVENT_LAST_ERRNO, MUGGLE_SYS_ERRNO_WOULDBLOCK);
			if (ret > 0) {
				w_n_bytes += ret;
			}
			break;
		}
		w_n_bytes += ret;
	}

	while (1) {
		ret = muggle_ev_fd_read(fds[1], buf, sizeof(buf));
		if (ret != (int)sizeof(buf)) {
			ASSERT_EQ(MUGGLE_EVENT_LAST_ERRNO, MUGGLE_SYS_ERRNO_WOULDBLOCK);
			if (ret >= (int)sizeof(int)) {
				*p = ++r_val;
			}
			if (ret > 0) {
				r_n_bytes += ret;
			}
			break;
		}
		*p = ++r_val;
		r_n_bytes += ret;
	}

	ASSERT_EQ(w_n_bytes, r_n_bytes);
}

TEST_F(TestEventFdFixture, shutdown)
{
	char buf[1024];
	muggle_ev_fd_shutdown(fds[1], MUGGLE_EVENT_FD_SHUT_RDWR);
	int ret = muggle_ev_fd_read(fds[0], buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
}
