/******************************************************************************
 *  @file         event_loop_poll.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-09
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event loop poll
 *****************************************************************************/

#ifndef MUGGLE_C_EVENT_LOOP_POLL_H_
#define MUGGLE_C_EVENT_LOOP_POLL_H_

#include "muggle/c/event/event_loop.h"

#if MUGGLE_PLATFORM_WINDOWS
#else
#include <poll.h>
#endif

EXTERN_C_BEGIN

typedef struct muggle_event_loop_poll
{
	muggle_event_loop_t base;  //!< base event loop

	struct pollfd *fds;    //!< pollfd array
	void          **nodes; //!< linked list node contain event context
	int           capcity; //!< pollfd array capacity
	int           nfd;     //!< number of open fd
} muggle_event_loop_poll_t;

MUGGLE_EV_LOOP_IMPL_DECLARE(poll)

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_LOOP_POLL_H_ */
