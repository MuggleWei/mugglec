#ifndef NET_FOO_DISPATCHER_H_
#define NET_FOO_DISPATCHER_H_

#include "foo/foo_macro.h"
#include "foo/foo_msg.h"

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
	int32_t           msg_len;  //!< length of message(include header), negative represent variable length
	foo_callback_func cb_func;  //!< callback function
}foo_callback_t;

typedef struct foo_dispatcher
{
	foo_callback_t callbacks[FOO_MSG_TYPE_MAX];  //!< callbacks
}foo_dispatcher_t;

/**
 * @brief initialize foo callbacks
 *
 * @param dispatcher   foo dispatcher
 */
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
bool foo_dispather_register(
	foo_dispatcher_t *dispatcher,
	uint32_t msg_type,
	int32_t msg_len,
	foo_callback_func callback);

/**
 * @brief dispatcher handle message
 *
 * @param ev         muggle socket event
 * @param peer       muggle socket peer
 *
 * @return boolean value
 */
bool foo_dispatcher_handle(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer);

EXTERN_C_BEGIN

#endif /* ifndef NET_FOO_DISPATCHER_H_ */
