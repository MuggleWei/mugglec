#ifndef FOO_CODEC_H_
#define FOO_CODEC_H_

#include "foo/foo_macro.h"

EXTERN_C_BEGIN

struct foo_codec;
struct foo_dispatcher;

/**
 * @brief prototype of codec function
 *
 * @param dispatcher message dispatcher
 * @param codec      codec object pointer
 * @param evloop     event loop
 * @param ctx        socket context
 * @param data       input/output data
 * @param data_len   length of data
 *
 * @return boolean value
 */
typedef bool (*foo_codec_func)(
	struct foo_dispatcher *dispatcher,
	struct foo_codec *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len);

typedef struct foo_codec
{
	struct foo_codec *prev;
	struct foo_codec *next;
	foo_codec_func   encode;
	foo_codec_func   decode;
} foo_codec_t;

EXTERN_C_END

#endif /* ifndef FOO_CODEC_H_ */
