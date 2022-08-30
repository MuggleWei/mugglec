#ifndef FOO_DISPATCHER_H_
#define FOO_DISPATCHER_H_

#include "foo/foo_macro.h"
#include "foo/foo_msg.h"
#include "foo/foo_codec.h"

EXTERN_C_BEGIN

typedef void (*foo_callback_func)(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *msg);

typedef struct foo_callback
{
	uint32_t          msg_type; //!< message type
	foo_callback_func cb_func;  //!< callback function
} foo_callback_t;

typedef struct foo_dispatcher
{
	foo_callback_t callbacks[FOO_MSG_TYPE_MAX]; //!< callbacks
	foo_codec_t    *codec_list_head;            //!< codec list head
	foo_codec_t    *codec_list_tail;            //!< codec list tail
} foo_dispatcher_t;

/**
 * @brief initialize foo dispatcher
 *
 * @param dispatcher  foo dispatcher
 *
 * @return boolean value
 */
NET_FOO_EXPORT
void foo_dispatcher_init(foo_dispatcher_t *dispatcher);

/**
 * @brief dispatcher append codec
 *
 * @param dispatcher  foo dispatcher
 * @param codec       foo codec
 *
 * @note codec order is the same as encode order
 */
NET_FOO_EXPORT
void foo_dispatcher_append_codec(foo_dispatcher_t *dispatcher, foo_codec_t *codec);

/**
 * @brief register message callback
 *
 * @param dispatcher  foo dispatcher
 * @param msg_type    message type
 * @param callback    callback function
 *
 * @return boolean value
 */
NET_FOO_EXPORT
bool foo_dispatcher_register(
	foo_dispatcher_t *dispatcher,
	uint32_t msg_type,
	foo_callback_func callback);

/**
 * @brief dispatcher handle message
 *
 * @param dispatcher  foo dispatcher
 * @param evloop      event loop
 * @param ctx         socket context
 *
 * @return boolean value
 */
NET_FOO_EXPORT
bool foo_dispatcher_handle(
	foo_dispatcher_t *dispatcher,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx);

/**
 * @brief dispatcher send message
 *
 * @param dispatcher  foo dispatcher
 * @param evloop      event loop
 * @param ctx         socket context
 * @param msg_type    message type
 * @param data        send data
 * @param data_len    length of data
 *
 * @return boolean value
 */
NET_FOO_EXPORT
bool foo_dispatcher_send(
	foo_dispatcher_t *dispatcher,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len);

/**
 * @brief decode handle
 *
 * @param dispatcher  foo dispatcher
 * @param codec       foo codec
 * @param evloop      event loop
 * @param ctx         socket context
 * @param data        data
 * @param data_len    length of data
 *
 * @return 
 */
NET_FOO_EXPORT
bool foo_dispatcher_decode(
	foo_dispatcher_t *dispatcher,
	foo_codec_t *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len);

/**
 * @brief encode handle
 *
 * @param dispatcher  foo dispatcher
 * @param codec       foo codec
 * @param evloop      event loop
 * @param ctx         socket context
 * @param data        data
 * @param data_len    length of data
 *
 * @return 
 */
NET_FOO_EXPORT
bool foo_dispatcher_encode(
	foo_dispatcher_t *dispatcher,
	foo_codec_t *codec,
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *data, uint32_t data_len);

EXTERN_C_END

#endif /* ifndef FOO_DISPATCHER_H_ */
