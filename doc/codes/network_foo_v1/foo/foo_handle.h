#ifndef FOO_HANDLE_H_
#define FOO_HANDLE_H_

#include "foo/foo_macro.h"
#include "foo/dispatcher.h"

EXTERN_C_BEGIN

/**
 * @brief foo event data in socket event
 */
typedef struct foo_ev_data
{
	foo_dispatcher_t  *dispatcher;     //!< message dispatcher
	uint32_t          recv_unit_size;  //!< number bytes of per read that from socket into bytes buffer
	uint32_t          msg_len_limit;   //!< max allowed message variable length, if 0, with no limit
	void              *user_data;      //!< user event data
}foo_ev_data_t;

/**
 * @brief foo user data in socket peer
 */
typedef struct foo_socket_peer_data
{
	char                  straddr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_bytes_buffer_t bytes_buf;
	void                  *user_data;
}foo_socket_peer_data_t;

/**
 * @brief foo on connection
 *
 * @param ev           muggle socket event
 * @param listen_peer  muggle socket listenr peer
 * @param peer         connection socket peer
 */
NET_FOO_EXPORT
void foo_on_connect(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *listen_peer,
	muggle_socket_peer_t *peer);

/**
 * @brief foo on message
 *
 * @param ev    muggle socket event
 * @param peer  muggle socket peer
 */
NET_FOO_EXPORT
void foo_on_message(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer);

/**
 * @brief foo on error
 *
 * @param ev    muggle socket event
 * @param peer  muggle socket peer
 */
NET_FOO_EXPORT
void foo_on_error(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer);

/**
 * @brief foo on close
 *
 * @param ev    muggle socket event
 * @param peer  muggle socket peer
 */
NET_FOO_EXPORT
void foo_on_close(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer);

/**
 * @brief send message 
 *
 * @param ev         muggle socket event
 * @param peer       muggle socket peer
 * @param data       send data
 * @param num_bytes  number of bytes in data
 */
NET_FOO_EXPORT
void foo_send(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *data,
	size_t num_bytes);

EXTERN_C_END

#endif /* ifndef FOO_HANDLE_H_ */
