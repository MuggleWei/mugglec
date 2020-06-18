/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_SOCKET_EVENT_SELECT_H_
#define MUGGLE_C_SOCKET_EVENT_SELECT_H_

#include "muggle/c/net/socket_event.h"

EXTERN_C_BEGIN

void muggle_socket_event_select(muggle_socket_event_t *ev, muggle_socket_ev_arg_t *ev_arg);

EXTERN_C_END

#endif
