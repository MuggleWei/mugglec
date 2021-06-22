/******************************************************************************
 *  @file         socket_event_poll.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket event - poll
 *****************************************************************************/

#ifndef MUGGLE_C_SOCKET_EVENT_POLL_H_
#define MUGGLE_C_SOCKET_EVENT_POLL_H_

#include "muggle/c/net/socket_event.h"

EXTERN_C_BEGIN

/**
 * @brief socket event - poll
 *
 * @param ev socket event
 */
void muggle_socket_event_poll(muggle_socket_event_t *ev);

EXTERN_C_END

#endif
