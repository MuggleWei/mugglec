#ifndef FOO_CODEC_BYTES_H_
#define FOO_CODEC_BYTES_H_

#include "foo/foo_macro.h"
#include "foo/foo_codec.h"
#include "foo/foo_dispatcher.h"

EXTERN_C_BEGIN

typedef foo_codec_t foo_codec_bytes_t;

/**
 * @brief  initalize codec bytes
 *
 * @param codec  codec bytes pointer
 */
NET_FOO_EXPORT
void foo_codec_bytes_init(foo_codec_bytes_t *codec);

NET_FOO_EXPORT
bool codec_bytes_encode(
	foo_dispatcher_t *dispatcher,
	struct foo_codec *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len);

NET_FOO_EXPORT
bool codec_bytes_decode(
	foo_dispatcher_t *dispatcher,
	struct foo_codec *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len);

EXTERN_C_END

#endif /* ifndef FOO_CODEC_BYTES_H_ */
