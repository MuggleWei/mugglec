#include "codec_endian.h"
#include "foo/foo_handle.h"
#include "foo/foo_msg.h"
#include "foo/codec/codec_endian_msg.h"

void foo_codec_endian_init(foo_codec_endian_t *codec)
{
	memset(&codec->codec, 0, sizeof(codec->codec));
	codec->codec.encode = codec_endian_encode;
	codec->codec.decode = codec_endian_decode;

	foo_endian_conv_t *convs = codec->conv;
	for (int i = 0; i < FOO_MSG_TYPE_MAX; i++)
	{
		memset(convs + i, 0, sizeof(foo_endian_conv_t));
	}

	// register endian converter
	foo_codec_endian_register(codec, FOO_MSG_TYPE_REQ_LOGIN, sizeof(foo_msg_req_login_t), enc_endian_req_login, dec_endian_req_login);
	foo_codec_endian_register(codec, FOO_MSG_TYPE_RSP_LOGIN, sizeof(foo_msg_rsp_login_t), enc_endian_rsp_login, dec_endian_rsp_login);
	foo_codec_endian_register(codec, FOO_MSG_TYPE_REQ_SUM, FOO_VARIABLE_LEN_MSG, enc_endian_req_sum, dec_endian_req_sum);
	foo_codec_endian_register(codec, FOO_MSG_TYPE_RSP_SUM, sizeof(foo_msg_rsp_sum_t), enc_endian_rsp_sum, dec_endian_rsp_sum);
}

bool foo_codec_endian_register(
	foo_codec_endian_t *codec,
	uint32_t msg_type,
	int32_t msg_len,
	foo_codec_endian_func encode,
	foo_codec_endian_func decode)
{
	if (msg_type >= FOO_MSG_TYPE_MAX)
	{
		MUGGLE_LOG_ERROR(
			"failed register endian codec, message type beyond max: msg_type=%lu",
			(unsigned long)msg_type);
		return false;
	}

	foo_endian_conv_t *convs = codec->conv;
	foo_endian_conv_t *conv = convs + msg_type;
	if (conv->msg_type != 0)
	{
		MUGGLE_LOG_ERROR(
			"failed register endian codec, duplicated message type: msg_type=%lu",
			(unsigned long)msg_type);
		return false;
	}

	conv->msg_type = msg_type;
	conv->msg_len = msg_len;
	conv->encode = encode;
	conv->decode = decode;

	return true;
}

bool codec_endian_encode(
	struct foo_codec *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len)
{
	foo_msg_header_t *p_header = (foo_msg_header_t*)data;
	if (p_header->msg_type >= FOO_MSG_TYPE_MAX)
	{
		MUGGLE_LOG_ERROR(
			"failed endian encode, invalid message type: msg_type=%lu",
			(unsigned long)p_header->msg_type);
		return false;
	}
	p_header->body_len = data_len - sizeof(foo_msg_header_t);

	foo_codec_endian_t *endian_codec = (foo_codec_endian_t*)codec;
	foo_endian_conv_t *conv = &endian_codec->conv[p_header->msg_type];
	if (conv->msg_type != 0)
	{
		if (conv->msg_len != FOO_VARIABLE_LEN_MSG && conv->msg_len != data_len)
		{
			MUGGLE_LOG_ERROR(
				"invalid endian encode, invalide message length: "
				"msg_type=%lu, data_len=%lu, expect_len=%lu",
				(unsigned long)p_header->msg_type, (unsigned long)data_len, (unsigned long)conv->msg_len);
			return false;
		}

		if (conv->encode)
		{
			if (conv->encode(data, data_len) == NULL)
			{
				MUGGLE_LOG_ERROR(
					"failed endian encode, encode function return NULL: msg_type=%lu",
					(unsigned long)p_header->msg_type);
				return false;
			}
		}
	}

	// convert header's endian
	p_header->msg_type = htonl(p_header->msg_type);
	p_header->body_len = htonl(p_header->body_len);

	return foo_dispatcher_encode_handle(codec->next, ev, peer, data, data_len);
}

bool codec_endian_decode(
	struct foo_codec *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len)
{
	foo_ev_data_t *ev_data = (foo_ev_data_t*)ev->datas;
	foo_dispatcher_t *dispatcher = ev_data->dispatcher;

	foo_socket_peer_data_t *peer_data = (foo_socket_peer_data_t*)peer->data;
	muggle_bytes_buffer_t *bytes_buf = &peer_data->bytes_buf;

	foo_codec_endian_t *endian_codec = (foo_codec_endian_t*)codec;

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
		if (msg_header.msg_type >= FOO_MSG_TYPE_MAX)
		{
			MUGGLE_LOG_ERROR("invalide message type: addr=%s, msg_type=%lu",
					peer_data->straddr, (unsigned long)msg_header.msg_type);
			return false;
		}

		// get endian converter
		foo_endian_conv_t *conv = &endian_codec->conv[msg_header.msg_type];
		if (conv->msg_type == 0)
		{
			MUGGLE_LOG_ERROR(
				"failed endian decode, without register endian codec: msg_type=%lu",
				(unsigned long)msg_header.msg_type);
			return false;
		}

		// check message length
		uint32_t total_bytes = (uint32_t)sizeof(foo_msg_header_t) + msg_header.body_len;
		if (conv->msg_len >= 0)
		{
			if (total_bytes != conv->msg_len)
			{
				MUGGLE_LOG_ERROR(
					"invalid message length: addr=%s, msg_type=%lu, recv_len=%lu, expect_len=%lu",
					peer_data->straddr, (unsigned long)msg_header.msg_type,
					(unsigned long)total_bytes, (unsigned long)conv->msg_len);
				return false;
			}
		}
		else if (ev_data->msg_len_limit != 0)
		{
			if (total_bytes > ev_data->msg_len_limit)
			{
				MUGGLE_LOG_ERROR(
					"invalid message length: addr=%s, msg_type=%lu, recv_len=%lu, msg_len_limit=%lu",
					peer_data->straddr, (unsigned long)msg_header.msg_type,
					(unsigned long)total_bytes, (unsigned long)ev_data->msg_len_limit);
				return false;
			}
		}
		else
		{
			// ignore message length check
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

			if (conv && conv->decode)
			{
				p_header = conv->decode(p_header, total_bytes);
			}

			bool ret = foo_dispatcher_decode_handle(codec->prev, ev, peer, p_header, total_bytes);

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

			if (conv && conv->decode)
			{
				p_header = conv->decode(p_header, total_bytes);
			}

			bool ret = foo_dispatcher_decode_handle(codec->prev, ev, peer, p_header, total_bytes);

			muggle_bytes_buffer_reader_move(bytes_buf, (int)total_bytes);

			if (!ret)
			{
				return false;
			}
		}
	}

	return true;
}

