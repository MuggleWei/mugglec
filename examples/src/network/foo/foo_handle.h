#ifndef FOO_HANDLE_H_
#define FOO_HANDLE_H_

#include "foo_config.h"
#include "foo_dispatcher.h"

EXTERN_C_BEGIN

typedef struct {
	foo_config_t *cfg; //!< config
	muggle_event_loop_t *evloop; //!< event loop
	muggle_socket_evloop_handle_t handle; //!< event loop handle
	muggle_time_counter_t timer_tc; //!< time counter
	int64_t timer_interval_ms; //!< timer interval
	int64_t read_idle_ms; //!< read idle milli-seconds
	int64_t write_idle_ms; //!< write idle milli-seconds
	muggle_time_wheel_t time_wheel; //!< time wheel
	muggle_memory_pool_t session_pool; //!< session pool
	foo_dispatcher_t dispatcher; //!< message dispatcher
} foo_handle_t;

/**
 * @brief initialize foo handle
 *
 * @param handle  foo handle
 * @param evloop  event loop
 * @param cfg     foo config
 *
 * @return boolean
 */
bool foo_handle_init(foo_handle_t *handle, muggle_event_loop_t *evloop,
					 foo_config_t *cfg);

/**
 * @brief destroy foo handle
 *
 * @param handle  foo handle
 */
void foo_handle_destroy(foo_handle_t *handle);

/**
 * @brief foo handle allocate session
 *
 * @param p  foo handle pointer
 *
 * @return session
 */
muggle_socket_context_t *foo_handle_alloc_session(void *p);

/**
 * @brief foo handle recycle session
 *
 * @param p     foo handle pointer
 * @param data  session
 */
void foo_handle_recycle_session(void *p, muggle_socket_context_t *data);

/**
 * @brief set socket options before connect
 *
 * @param fd  socket fd
 * @param p   user data pointer
 */
void foo_set_socket_opt_before_conn(muggle_socket_t fd, void *p);

/**
 * @brief set socket options after connect
 *
 * @param fd  socket fd
 * @param p   user data pointer
 */
void foo_set_socket_opt_after_conn(muggle_socket_t fd, void *p);

/**
 * @brief foo handle message
 *
 * @param handle   foo handle
 * @param session  session
 */
void foo_handle_on_message(foo_handle_t *handle, foo_session_t *session);

/**
 * @brief foo decode message and dispatcher
 *
 * @param handle     foo handle
 * @param session    session
 * @param bytes_buf  bytes buffer
 */
void foo_handle_msg_decode_dispatch(foo_handle_t *handle,
									foo_session_t *session,
									muggle_bytes_buffer_t *bytes_buf);

/**
 * @brief foo encode message and send
 *
 * @param session  session
 * @param hdr      message hdr
 * @param datalen  total bytes of message
 *
 * @return number of bytes writed
 */
int foo_handle_msg_encode_send(foo_session_t *session, foo_msg_hdr_t *hdr,
							   uint32_t datalen);

/**
 * @brief handle time wheel update
 *
 * @param node       time wheel node
 * @param user_data  user data (foo_handle_t*)
 */
void foo_time_wheel_update(muggle_time_wheel_node_t *node, void *user_data);

/**
 * @brief on write idle
 *
 * @param session  session
 */
void foo_on_write_idle(foo_session_t *session);

/**
 * @brief on read idle
 *
 * @param session  session
 */
void foo_on_read_idle(foo_session_t *session);

EXTERN_C_END

#endif // !FOO_HANDLE_H_
