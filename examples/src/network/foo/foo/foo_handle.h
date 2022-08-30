#ifndef FOO_HANDLE_H_
#define FOO_HANDLE_H_

#include "foo/foo_macro.h"
#include "foo/foo_dispatcher.h"

EXTERN_C_BEGIN

#define CONN_NUM_LIMIT 32

/**
 * @brief event loop data
 */
typedef struct foo_evloop_data
{
	muggle_linked_list_t conn_list;      //!< connection list
	foo_dispatcher_t     dispatcher;     //!< message dispatcher
	uint32_t             recv_unit_size; //!< number bytes of per read
	uint32_t             msg_len_limit;  //!< max allowed message variable length
	void                 *user_data;
} foo_evloop_data_t;

typedef struct foo_socket_ctx_data
{
	uint32_t                  user_id;
	char                      straddr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_bytes_buffer_t     bytes_buf;
	uint64_t                  last_sec;
	muggle_linked_list_node_t *conn_node;
	void                      *user_data;
} foo_socket_ctx_data_t;

NET_FOO_EXPORT
void foo_handle_init(muggle_event_loop_t *evloop, void *user_data);

NET_FOO_EXPORT
void foo_handle_destroy(muggle_event_loop_t *evloop);

NET_FOO_EXPORT
foo_dispatcher_t* foo_handle_dipatcher(muggle_event_loop_t *evloop);

NET_FOO_EXPORT
muggle_linked_list_t* foo_handle_conn_list(muggle_event_loop_t *evloop);

NET_FOO_EXPORT
void foo_on_connect(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

NET_FOO_EXPORT
void foo_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

NET_FOO_EXPORT
void foo_on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

EXTERN_C_END

#endif /* ifndef FOO_HANDLE_H_ */
