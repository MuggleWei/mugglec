#include "codec_bytes.h"
#include "foo/foo_handle.h"
#include "foo/foo_msg.h"

foo_codec_t* codec_bytes_instance()
{
	static foo_codec_t codec_bytes = {
		NULL, NULL,
		codec_bytes_encode,
		codec_bytes_decode
	};
	return &codec_bytes;
}

void foo_codec_bytes_init(foo_codec_t *codec)
{
	memset(codec, 0, sizeof(*codec));
	codec->encode = codec_bytes_encode;
	codec->decode = codec_bytes_decode;
}

bool codec_bytes_encode(
	struct foo_codec *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len)
{
	// do nothing
	return foo_dispatcher_encode_handle(codec->next, ev, peer, data, data_len);
}

bool codec_bytes_decode(
	struct foo_codec *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len)
{
	foo_ev_data_t *ev_data = (foo_ev_data_t*)ev->datas;
	foo_dispatcher_t *dispatcher = ev_data->dispatcher;

	foo_socket_peer_data_t *peer_data = (foo_socket_peer_data_t*)peer->data;
	muggle_bytes_buffer_t *bytes_buf = &peer_data->bytes_buf;

	// read bytes into bytes buffer
	while (1)
	{
		void *p = muggle_bytes_buffer_writer_fc(bytes_buf, ev_data->recv_unit_size);
		if (p == NULL)
		{
			MUGGLE_LOG_WARNING("bytes buffer full: %s", peer_data->straddr);
			break;
		}

		int n = muggle_socket_peer_recv(peer, p, ev_data->recv_unit_size, 0);
		if (n > 0)
		{
			muggle_bytes_buffer_writer_move(bytes_buf, n);
		}

		if (n < (int)ev_data->recv_unit_size)
		{
			break;
		}
	}

	return foo_dispatcher_decode_handle(codec->prev, ev, peer, data, data_len);
}
