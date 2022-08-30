#include "codec_endian.h"
#include "foo/codec/codec_endian_msg.h"
#include "foo/foo_handle.h"

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
	foo_codec_endian_register(codec, FOO_MSG_TYPE_PING, sizeof(foo_msg_ping_t), enc_endian_ping, dec_endian_ping);
	foo_codec_endian_register(codec, FOO_MSG_TYPE_PONG, sizeof(foo_msg_pong_t), enc_endian_pong, dec_endian_pong);
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
	foo_dispatcher_t *dispatcher,
	struct foo_codec *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len)
{
	foo_msg_header_t *p_header = (foo_msg_header_t*)data;
	if (p_header->msg_type >= FOO_MSG_TYPE_MAX)
	{
		LOG_ERROR(
			"failed endian encode, invalid message type: msg_type=%lu",
			(unsigned long)p_header->msg_type);
		return false;
	}
	p_header->body_len = data_len - sizeof(foo_msg_header_t);

	foo_codec_endian_t *endian_codec = (foo_codec_endian_t*)codec;
	foo_endian_conv_t *conv = &endian_codec->conv[p_header->msg_type];
	if (conv->msg_type != 0)
	{
		if (conv->msg_len != FOO_VARIABLE_LEN_MSG && conv->msg_len != (int32_t)data_len)
		{
			LOG_ERROR(
				"invalid endian encode, invalide message length: "
				"msg_type=%lu, data_len=%lu, expect_len=%lu",
				(unsigned long)p_header->msg_type, (unsigned long)data_len, (unsigned long)conv->msg_len);
			return false;
		}

		if (conv->encode)
		{
			if (conv->encode(data, data_len) == NULL)
			{
				LOG_ERROR(
					"failed endian encode, encode function return NULL: msg_type=%lu",
					(unsigned long)p_header->msg_type);
				return false;
			}
		}
	}

	return foo_dispatcher_encode(dispatcher, codec->next, evloop, ctx, data, data_len);
}

bool codec_endian_decode(
	foo_dispatcher_t *dispatcher,
	struct foo_codec *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len)
{
	foo_msg_header_t *p_header = (foo_msg_header_t*)data;
	foo_codec_endian_t *endian_codec = (foo_codec_endian_t*)codec;
	foo_socket_ctx_data_t *ctx_data = (foo_socket_ctx_data_t*)muggle_socket_ctx_get_data(ctx);

	// check message type
	if (p_header->msg_type >= FOO_MSG_TYPE_MAX || p_header->msg_type <= 0)
	{
		LOG_ERROR("invalide message type: addr=%s, msg_type=%lu",
			ctx_data->straddr, (unsigned long)p_header->msg_type);
		return false;
	}

	// get endian converter
	foo_endian_conv_t *conv = &endian_codec->conv[p_header->msg_type];
	if (conv->msg_type == 0)
	{
		LOG_ERROR(
			"failed endian decode, without register endian codec: msg_type=%lu",
			(unsigned long)p_header->msg_type);
		return false;
	}

	// check message length
	uint32_t total_bytes = (uint32_t)data_len;
	if (conv->msg_len >= 0 && total_bytes != (uint32_t)conv->msg_len)
	{
		LOG_ERROR(
			"invalid message length: addr=%s, msg_type=%lu, recv_len=%lu, expect_len=%lu",
			ctx_data->straddr, (unsigned long)p_header->msg_type,
			(unsigned long)total_bytes, (unsigned long)conv->msg_len);
		return false;
	}

	if (conv->decode)
	{
		if (conv->decode(data, data_len) == NULL)
		{
			LOG_ERROR(
				"failed endian decode, decode function return NULL: msg_type=%lu",
				(unsigned long)p_header->msg_type);
			return false;
		}
	}

	return foo_dispatcher_decode(dispatcher, codec->prev, evloop, ctx, p_header, total_bytes);
}
