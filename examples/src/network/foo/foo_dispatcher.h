#ifndef FOO_DISPATCHER_H_
#define FOO_DISPATCHER_H_

#include "foo_config.h"
#include "foo_msg.h"
#include "foo_session.h"

EXTERN_C_BEGIN

typedef void (*fn_foo_callback)(muggle_event_loop_t *evloop,
								foo_session_t *session, foo_msg_hdr_t *hdr,
								void *data);

typedef struct {
	fn_foo_callback default_callback; //!< default callback
	fn_foo_callback callbacks[MAX_FOO_MSG_ID]; //!< message callbacks
} foo_dispatcher_t;

bool foo_dispatcher_init(foo_dispatcher_t *dispatcher);

void foo_dispatcher_destroy(foo_dispatcher_t *dispatcher);

bool foo_dispatcher_register_default_cb(foo_dispatcher_t *dispatcher,
										fn_foo_callback cb);

bool foo_dispatcher_register(foo_dispatcher_t *dispatcher, uint32_t msg_id,
							 fn_foo_callback cb);

void foo_dispatcher_dispatch(foo_dispatcher_t *dispatcher,
							 muggle_event_loop_t *evloop,
							 foo_session_t *session, foo_msg_hdr_t *hdr);

EXTERN_C_END

#endif // !FOO_DISPATCHER_H_
