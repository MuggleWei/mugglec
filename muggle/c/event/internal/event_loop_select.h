/******************************************************************************
 *  @file         event_loop_select.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-09
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event loop select
 *****************************************************************************/

#ifndef MUGGLE_C_EVENT_LOOP_SELECT_H_
#define MUGGLE_C_EVENT_LOOP_SELECT_H_

#include "muggle/c/event/event_loop.h"

#if MUGGLE_PLATFORM_WINDOWS
#else
#include <sys/select.h>
#endif

EXTERN_C_BEGIN

typedef struct muggle_event_loop_select
{
	muggle_event_loop_t base;  //!< base event loop

	fd_set allset; //!< all fd set
	int    nfds;   //!< highest-numbered file descriptor, use *nix
} muggle_event_loop_select_t;

MUGGLE_EV_LOOP_IMPL_DECLARE(select)

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_LOOP_SELECT_H_ */

