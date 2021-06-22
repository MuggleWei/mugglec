/******************************************************************************
 *  @file         socket_event_epoll.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket event - epoll
 *****************************************************************************/

#ifndef MUGGLE_C_SOCKET_EVENT_EPOLL_H_
#define MUGGLE_C_SOCKET_EVENT_EPOLL_H_

#include "muggle/c/net/socket_event.h"

#if MUGGLE_PLATFORM_LINUX

EXTERN_C_BEGIN

/**
 * @brief socket event - epoll
 *
 * @param ev socket event
 */
void muggle_socket_event_epoll(muggle_socket_event_t *ev);

EXTERN_C_END

#endif

#endif
