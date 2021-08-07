#ifndef NET_FOO_CODEC_H_
#define NET_FOO_CODEC_H_

#include "foo/foo_macro.h"

EXTERN_C_BEGIN

struct foo_codec;

/**
 * @brief prototype of encode function
 *
 * @param ev        muggle socket event
 * @param peer      muggle socket peer
 * @param data      data from previous codec
 * @param data_len  input data length
 * @param out_len   output data length
 *
 * @return 
 *     - on success, return message of this encode
 *     - on failed, return NULL
 */
typedef bool (*foo_codec_func)(
	struct foo_codec *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len);

typedef struct foo_codec
{
	struct foo_codec     *prev;
	struct foo_codec     *next;
	foo_codec_func       encode;
	foo_codec_func       decode;
}foo_codec_t;

EXTERN_C_END

#endif /* ifndef NET_FOO_CODEC_H_ */
