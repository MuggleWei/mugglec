#ifndef FOO_CODEC_ENDIAN_H_
#define FOO_CODEC_ENDIAN_H_

#include "foo/foo_macro.h"
#include "foo/foo_codec.h"
#include "foo/foo_dispatcher.h"

EXTERN_C_BEGIN

/**
 * @brief prototype of codec endian function
 *
 * @param data      data
 * @param data_len  data length
 *
 * @return 
 *     - on success, return result of encode/decode
 *     - on failed, return NULL
 */
typedef void* (*foo_codec_endian_func)(void *data, uint32_t data_len);

typedef struct foo_endian_conv
{
	uint32_t              msg_type;  //!< message type
	int32_t               msg_len;   //!< length of message(include header), negative represent variable length
	foo_codec_endian_func encode;    //!< encode function
	foo_codec_endian_func decode;    //!< decode function
}foo_endian_conv_t;

typedef struct foo_codec_endian
{
	foo_codec_t        codec;
	foo_endian_conv_t  conv[FOO_MSG_TYPE_MAX];
}foo_codec_endian_t;

/**
 * @brief  initialize codec endian
 *
 * @param codec  codec endian pointer
 */
NET_FOO_EXPORT
void foo_codec_endian_init(foo_codec_endian_t *codec);

/**
 * @brief register encode/decode function for message type
 *
 * @param codec     endian codec
 * @param msg_type  message type
 * @param msg_len   length of message
 * @param encode    encode function
 * @param decode    decode function
 *
 * @return boolean value
 */
NET_FOO_EXPORT
bool foo_codec_endian_register(
	foo_codec_endian_t *codec,
	uint32_t msg_type,
	int32_t msg_len,
	foo_codec_endian_func encode,
	foo_codec_endian_func decode);

NET_FOO_EXPORT
bool codec_endian_encode(
	foo_dispatcher_t *dispatcher,
	struct foo_codec *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len);

NET_FOO_EXPORT
bool codec_endian_decode(
	foo_dispatcher_t *dispatcher,
	struct foo_codec *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len);

EXTERN_C_END

#endif /* ifndef FOO_CODEC_ENDIAN_H_ */
