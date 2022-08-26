#ifndef EXAMPLE_DATA_STRUCT_H_
#define EXAMPLE_DATA_STRUCT_H_

#include "muggle/c/muggle_c.h"

typedef struct evloop_data
{
	char msg[128];
} evloop_data_t;

typedef struct ev_data
{
	fn_muggle_evloop_cb1 cb_read;  //!< on event context read callback
	fn_muggle_evloop_cb1 cb_close; //!< on event context close callback
	fn_muggle_evloop_cb2 cb_wake;  //!< on event loop wakeup callback
	fn_muggle_evloop_cb2 cb_timer; //!< on event loop timer callback
	fn_muggle_evloop_cb1 cb_clear; //!< on event loop exit soon, foreach clear context callback
} ev_data_t;

#endif