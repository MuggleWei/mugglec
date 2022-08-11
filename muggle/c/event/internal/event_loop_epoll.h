/******************************************************************************
 *  @file         event_loop_epoll.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-09
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event loop epoll
 *****************************************************************************/

#ifndef MUGGLE_C_EVENT_LOOP_EPOLL_H_
#define MUGGLE_C_EVENT_LOOP_EPOLL_H_

#include "muggle/c/event/event_loop.h"
#if MUGGLE_PLATFORM_LINUX
#include <sys/epoll.h>

EXTERN_C_BEGIN

typedef struct muggle_event_loop_epoll
{
	muggle_event_loop_t base;  //!< base event loop

	muggle_event_fd    epfd;     //!< epoll fd
	struct epoll_event *events;  //!< use for wait events
	int                capacity; //!< events array capacity
} muggle_event_loop_epoll_t;

MUGGLE_EV_LOOP_IMPL_DECLARE(epoll)

#endif

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_LOOP_EPOLL_H_ */
