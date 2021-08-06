#ifndef NET_FOO_DISPATCHER_H_
#define NET_FOO_DISPATCHER_H_

#include "foo/foo_macro.h"
#include "foo/foo_msg.h"
#include "foo/codec.h"

EXTERN_C_BEGIN

/**
 * @brief prototype of foo callback function
 *
 * @param ev    muggle socket event
 * @param peer  muggle socket peer
 * @param msg   message pointer
 *
 * @return 
 */
typedef void (*foo_callback_func)(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg);

typedef struct foo_callback
{
	uint32_t          msg_type; //!< message type
	foo_callback_func cb_func;  //!< callback function
}foo_callback_t;

typedef struct foo_dispatcher
{
	foo_callback_t  callbacks[FOO_MSG_TYPE_MAX];  //!< callbacks
	foo_codec_t     *codec_list_head;             //!< codec list head
	foo_codec_t     *codec_list_tail;             //!< codec list tail
}foo_dispatcher_t;

/**
 * @brief initialize foo callbacks
 *
 * @param dispatcher   foo dispatcher
 */
NET_FOO_EXPORT
void foo_dispatcher_init(foo_dispatcher_t *dispatcher);

/**
 * @brief register callback and codec functions
 *
 * @param dispatcher   foo dispatcher
 * @param msg_type     message type
 * @param msg_len      length of message(include header)
 * @param callback     callback function
 *
 * @return boolean value
 */
NET_FOO_EXPORT
bool foo_dispather_register(
	foo_dispatcher_t *dispatcher,
	uint32_t msg_type,
	foo_callback_func callback);

/**
 * @brief dispatcher append codec
 *
 * NOTE: codec order is the same as encode order
 *
 * @param dispatcher foo dispatcher
 * @param codec      foo codec
 */
NET_FOO_EXPORT
void foo_dispatcher_append_codec(
	foo_dispatcher_t *dispatcher,
	foo_codec_t *codec);

/**
 * @brief dispatcher handle message
 *
 * @param ev         muggle socket event
 * @param peer       muggle socket peer
 *
 * @return boolean value
 */
NET_FOO_EXPORT
bool foo_dispatcher_handle(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer);

/**
 * @brief dispatcher send message
 *
 * @param ev        muggle socket event
 * @param peer      muggle socket peer
 * @param data      data
 * @param data_len  length of data
 *
 * @return 
 */
NET_FOO_EXPORT
bool foo_dispatcher_send(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len);

/**
 * @brief encode handle
 *
 * @param codec     foo codec
 * @param ev        muggle socket event
 * @param peer      muggle socket peer
 * @param data      data from previous codec
 * @param data_len  length of data
 *
 * @return 
 */
NET_FOO_EXPORT
bool foo_dispatcher_encode_handle(
	foo_codec_t *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len);

/**
 * @brief encode handle
 *
 * @param codec     foo codec
 * @param ev        muggle socket event
 * @param peer      muggle socket peer
 * @param data      data from previous codec
 * @param data_len  length of data
 *
 * @return 
 */
NET_FOO_EXPORT
bool foo_dispatcher_decode_handle(
	foo_codec_t *codec,
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data, uint32_t data_len);

EXTERN_C_BEGIN

#endif /* ifndef NET_FOO_DISPATCHER_H_ */
