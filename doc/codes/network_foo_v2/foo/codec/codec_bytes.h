#ifndef NET_FOO_CODEC_BYTES_H_
#define NET_FOO_CODEC_BYTES_H_

#include "foo/foo_macro.h"
#include "foo/codec.h"

EXTERN_C_BEGIN

/**
 * @brief  initalize codec bytes
 *
 * @param codec  codec bytes pointer
 */
NET_FOO_EXPORT
void foo_codec_bytes_init(foo_codec_t *codec);

NET_FOO_EXPORT
bool codec_bytes_encode(
	struct foo_codec *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len);

NET_FOO_EXPORT
bool codec_bytes_decode(
	struct foo_codec *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len);

EXTERN_C_END

#endif /* ifndef NET_FOO_CODEC_BYTES_H_ */
