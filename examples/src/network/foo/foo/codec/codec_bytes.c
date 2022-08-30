#include "codec_bytes.h"
#include "foo/foo_handle.h"

void foo_codec_bytes_init(foo_codec_bytes_t *codec)
{
	memset(codec, 0, sizeof(*codec));

	codec->encode = codec_bytes_encode;
	codec->decode = codec_bytes_decode;
}

bool codec_bytes_encode(
	foo_dispatcher_t *dispatcher,
	struct foo_codec *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len)
{
	foo_msg_header_t *header = (foo_msg_header_t*)data;
	header->msg_type = htonl(header->msg_type);
	header->body_len = data_len - sizeof(foo_msg_header_t);
	header->body_len = htonl(header->body_len);

	return foo_dispatcher_encode(dispatcher, codec->next, evloop, ctx, data, data_len);
}

bool codec_bytes_decode(
	foo_dispatcher_t *dispatcher,
	struct foo_codec *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len)
{
	foo_evloop_data_t *evloop_data = (foo_evloop_data_t*)muggle_evloop_get_data(evloop);

	foo_socket_ctx_data_t *ctx_data = (foo_socket_ctx_data_t*)muggle_socket_ctx_get_data(ctx);
	muggle_bytes_buffer_t *bytes_buf = &ctx_data->bytes_buf;

	// read bytes into bytes buffer
	while (1)
	{
		void *p = muggle_bytes_buffer_writer_fc(bytes_buf, evloop_data->recv_unit_size);
		if (p == NULL)
		{
			LOG_WARNING("bytes buffer full: %s", ctx_data->straddr);
			break;
		}

		int n = muggle_socket_ctx_read(ctx, p, evloop_data->recv_unit_size);
		if (n > 0)
		{
			muggle_bytes_buffer_writer_move(bytes_buf, n);
		}

		if (n < (int)evloop_data->recv_unit_size)
		{
			break;
		}
	}

	// parse message
	foo_msg_header_t msg_header;
	while (1)
	{
		if (!muggle_bytes_buffer_fetch(bytes_buf, (int)sizeof(msg_header), &msg_header))
		{
			// readable bytes less than sizeof message header
			break;
		}

		// convert header's endian
		msg_header.msg_type = ntohl(msg_header.msg_type);
		msg_header.body_len = ntohl(msg_header.body_len);

		// check message type
		if (msg_header.msg_type >= FOO_MSG_TYPE_MAX || msg_header.msg_type <= 0)
		{
			LOG_ERROR("invalide message type: addr=%s, msg_type=%lu",
				ctx_data->straddr, (unsigned long)msg_header.msg_type);
			return false;
		}

		// check message length
		uint32_t total_bytes = (uint32_t)sizeof(foo_msg_header_t) + msg_header.body_len;
		if (evloop_data->msg_len_limit > 0)
		{
			if (total_bytes > evloop_data->msg_len_limit)
			{
				LOG_ERROR(
					"invalid message length: addr=%s, msg_type=%lu, recv_len=%lu, msg_len_limit=%lu",
					ctx_data->straddr, (unsigned long)msg_header.msg_type,
					(unsigned long)total_bytes, (unsigned long)evloop_data->msg_len_limit);
				return false;
			}
		}

		// check readable
		int readable = muggle_bytes_buffer_readable(bytes_buf);
		if ((uint32_t)readable < total_bytes)
		{
			break;
		}

		// handle message
		foo_msg_header_t *p_header =
			(foo_msg_header_t*)muggle_bytes_buffer_reader_fc(bytes_buf, (uint32_t)total_bytes);
		if (p_header == NULL)
		{
			// discontinuous memory
			void *buf = malloc(total_bytes);
			muggle_bytes_buffer_read(bytes_buf, (int)total_bytes, buf);

			// convert header's endian
			p_header = (foo_msg_header_t*)buf;
			p_header->msg_type = msg_header.msg_type;
			p_header->body_len = msg_header.body_len;

			bool ret = foo_dispatcher_decode(dispatcher, codec->prev, evloop, ctx, p_header, total_bytes);

			free(buf);

			if (!ret)
			{
				return false;
			}
		}
		else
		{
			// continuous memory

			// convert header's endian
			p_header->msg_type = msg_header.msg_type;
			p_header->body_len = msg_header.body_len;

			bool ret = foo_dispatcher_decode(dispatcher, codec->prev, evloop, ctx, p_header, total_bytes);

			muggle_bytes_buffer_reader_move(bytes_buf, (int)total_bytes);

			if (!ret)
			{
				return false;
			}
		}
	}
	
	return true;
}
