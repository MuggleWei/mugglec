#include "muggle/c/muggle_c.h"

;
#pragma pack(push, 1)

typedef struct {
	char magic[4];
	int32_t payloadlen;
	uint32_t reversed2;
	uint32_t reversed3;
} msg_hdr_t;

typedef struct {
	uint64_t reserved;
} msg_tail_t;

typedef struct {
	char data[101];
} msg_foo_t;

#pragma pack(pop)

#define N 1000

#define N_BODY 100
#define N_TAIL 64

static const char *s_magic = "#XYZ";
const uint64_t s_tail = 0x0102030405060708;

bool check_tail(void *p)
{
	msg_hdr_t *hdr = (msg_hdr_t *)p;
	msg_tail_t *tail = (msg_tail_t *)((char *)(hdr + 1) + hdr->payloadlen);
	if (tail->reserved != s_tail) {
		return false;
	}

	return true;
}

void read_datas(muggle_bytes_buffer_t *bytes_buf)
{
	msg_hdr_t msg_hdr;
	while (1) {
		if (!muggle_bytes_buffer_fetch(bytes_buf, (int)sizeof(msg_hdr_t),
									   &msg_hdr)) {
			break;
		}

		// check magic word
		if (memcmp(msg_hdr.magic, s_magic, 4) != 0) {
			LOG_FATAL("invalid magic word: c=%d, w=%d, r=%d, t=%d",
					  bytes_buf->c, bytes_buf->w, bytes_buf->r, bytes_buf->t);
		}

		// check readable
		int total_bytes = (int)sizeof(msg_hdr_t) + msg_hdr.payloadlen +
						  (int)sizeof(msg_tail_t);
		int readable = muggle_bytes_buffer_readable(bytes_buf);
		if (readable < total_bytes) {
			break;
		}

		// handle message
		msg_hdr_t *p_msg =
			(msg_hdr_t *)muggle_bytes_buffer_reader_fc(bytes_buf, total_bytes);
		if (p_msg == NULL) {
			// discontinuous memory
			void *buf = malloc(total_bytes);

			muggle_bytes_buffer_read(bytes_buf, total_bytes, buf);

			if (!check_tail(buf)) {
				LOG_FATAL("invalid tail: c=%d, w=%d, r=%d, t=%d", bytes_buf->c,
						  bytes_buf->w, bytes_buf->r, bytes_buf->t);
			}

			free(buf);

			LOG_INFO("read discontinuous: c=%d, w=%d, r=%d, t=%d", bytes_buf->c,
					 bytes_buf->w, bytes_buf->r, bytes_buf->t);
		} else {
			// continuous memory
			if (!check_tail(p_msg)) {
				LOG_FATAL("invalid tail: c=%d, w=%d, r=%d, t=%d", bytes_buf->c,
						  bytes_buf->w, bytes_buf->r, bytes_buf->t);
			}

			muggle_bytes_buffer_reader_move(bytes_buf, total_bytes);

			LOG_INFO("read continuous: c=%d, w=%d, r=%d, t=%d", bytes_buf->c,
					 bytes_buf->w, bytes_buf->r, bytes_buf->t);
		}
	}
}

void run(muggle_bytes_buffer_t *bytes_buf, char *datas, int total_bytes)
{
	while (1) {
		int remain = total_bytes;
		char *data = datas;
		while (remain > 0) {
			int n = rand() % 200 + N_BODY * sizeof(msg_foo_t);
			if (n > remain) {
				n = remain;
			}

			// write datas
			void *p = muggle_bytes_buffer_writer_fc(bytes_buf, n);
			if (p == NULL) {
				LOG_INFO("exit test");
				exit(EXIT_SUCCESS);
			}

			int real_n = n / 2;
			memcpy(p, data, real_n);
			// muggle_bytes_buffer_writer_move(bytes_buf, real_n);
			muggle_bytes_buffer_writer_move_n(bytes_buf, p, real_n);

			LOG_INFO("write: n=%d, c=%d, w=%d, r=%d, t=%d", n, bytes_buf->c,
					 bytes_buf->w, bytes_buf->r, bytes_buf->t);

			remain -= real_n;
			data += real_n;

			// read datas
			read_datas(bytes_buf);
		}
	}
}

void init_log(const char *filepath)
{
	static muggle_log_file_rotate_handler_t file_rot_handler;

	int ret = muggle_log_file_rotate_handler_init(&file_rot_handler, filepath,
												  16 * 1024 * 1024, 5);
	if (ret != 0) {
		fprintf(stderr, "failed open file: %s\n", filepath);
		return;
	}
	muggle_log_handler_set_level((muggle_log_handler_t *)&file_rot_handler,
								 LOG_LEVEL_DEBUG);

	static muggle_log_console_handler_t console_handler;
	muggle_log_console_handler_init(&console_handler, 1);
	muggle_log_handler_set_level((muggle_log_handler_t *)&console_handler,
								 LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t *)&file_rot_handler);
	// logger->add_handler(logger, (muggle_log_handler_t *)&console_handler);
}

int main()
{
	// init log
	init_log("logs/try_bytes_buffer.log");

	// init bytes buffer
	muggle_bytes_buffer_t bytes_buf;
	muggle_bytes_buffer_init(&bytes_buf, 2 * 1024 * 1024);

	// prepared datas
	size_t data_sz =
		sizeof(msg_hdr_t) + N_BODY * sizeof(msg_foo_t) + sizeof(msg_tail_t);
	size_t end_data_sz =
		sizeof(msg_hdr_t) + N_TAIL * sizeof(msg_foo_t) + sizeof(msg_tail_t);
	char *datas = malloc(data_sz * N + end_data_sz);

	// write datas
	msg_hdr_t *p = (msg_hdr_t *)datas;
	for (int i = 0; i < N; ++i) {
		memcpy(p, s_magic, 4);
		p->payloadlen = N_BODY * sizeof(msg_foo_t);

		msg_foo_t *body = (msg_foo_t *)(p + 1);
		memset(body, 0, sizeof(*body) * N_BODY);

		msg_tail_t *tail = (msg_tail_t *)(body + N_BODY);
		tail->reserved = s_tail;

		p = (msg_hdr_t *)(tail + 1);
	}

	memcpy(p, s_magic, 4);
	p->payloadlen = N_TAIL * sizeof(msg_foo_t);

	msg_foo_t *body = (msg_foo_t *)(p + 1);
	memset(body, 0, sizeof(*body) * N_TAIL);

	msg_tail_t *tail = (msg_tail_t *)(body + N_TAIL);
	tail->reserved = s_tail;

	p = (msg_hdr_t *)(tail + 1);

	// tests
	srand(time(NULL));
	run(&bytes_buf, datas, data_sz * N + end_data_sz);

	// free datas
	free(datas);

	// destroy bytes buffer
	muggle_bytes_buffer_destroy(&bytes_buf);

	return 0;
}
