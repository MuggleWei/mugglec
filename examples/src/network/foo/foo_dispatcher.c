#include "foo_dispatcher.h"

bool foo_dispatcher_init(foo_dispatcher_t *dispatcher)
{
	memset(dispatcher, 0, sizeof(foo_dispatcher_t));
	return true;
}

void foo_dispatcher_destroy(foo_dispatcher_t *dispatcher)
{
	MUGGLE_UNUSED(dispatcher);
}

bool foo_dispatcher_register(foo_dispatcher_t *dispatcher, uint32_t msg_id,
							 fn_foo_callback cb)
{
	if (msg_id <= 0 || msg_id >= MAX_FOO_MSG_ID) {
		LOG_ERROR("invalid message id: %u", msg_id);
		return false;
	}

	if (dispatcher->callbacks[msg_id] != NULL) {
		LOG_ERROR("repeated register message: %u", msg_id);
		return false;
	}

	dispatcher->callbacks[msg_id] = cb;

	return true;
}

bool foo_dispatcher_register_default_cb(foo_dispatcher_t *dispatcher,
										fn_foo_callback cb)
{
	if (dispatcher->default_callback != NULL) {
		LOG_ERROR("repeated register default callback");
		return false;
	}

	dispatcher->default_callback = cb;

	return true;
}

void foo_dispatcher_dispatch(foo_dispatcher_t *dispatcher,
							 muggle_event_loop_t *evloop,
							 foo_session_t *session, foo_msg_hdr_t *hdr)
{
	if (hdr->msg_id <= 0 || hdr->msg_id > MAX_FOO_MSG_ID) {
		LOG_ERROR("invalid message id: %u", hdr->msg_id);
		foo_session_shutdown(session);
		return;
	}

	fn_foo_callback cb = dispatcher->callbacks[hdr->msg_id];
	if (cb) {
		cb(evloop, session, hdr, hdr + 1);
	} else {
		if (dispatcher->default_callback) {
			dispatcher->default_callback(evloop, session, hdr, hdr + 1);
		} else {
			LOG_ERROR("unhandle message: %u", hdr->msg_id);
		}
	}
}
