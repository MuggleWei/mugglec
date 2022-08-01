/******************************************************************************
 *  @file         event_signal.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-01
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event signal
 *****************************************************************************/

#ifndef MUGGLE_C_EVENT_SIGNAL_H_
#define MUGGLE_C_EVENT_SIGNAL_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/event/event.h"

EXTERN_C_BEGIN

typedef struct muggle_event_signal
{
#if MUGGLE_PLATFORM_LINUX
	muggle_event_fd evfd;  //!< eventfd
#elif MUGGLE_PLATFORM_WINDOWS
	muggle_event_fd socket_fds[2]; //!< socket fds
	muggle_mutex_t  *mtx;          //!< socket write mutex
#else
	muggle_event_fd pipe_fds[2];  //!< pipe fds
	muggle_mutex_t  *mtx;         //!< pipe write mutex
#endif
} muggle_event_signal_t;

/**
 * @brief init event signal
 *
 * @param ev_signal  event signal
 *
 * @return 
 *     0 - success
 *     otherwise - return MUGGLE_EVENT_ERROR and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_event_signal_init(muggle_event_signal_t *ev_signal);

/**
 * @brief destroy event signal
 *
 * @param ev_signal  event signal
 */
MUGGLE_C_EXPORT
void muggle_event_signal_destroy(muggle_event_signal_t *ev_signal);

/**
 * @brief event signal wake up
 *
 * @param ev_signal  event signal
 *
 * @return 
 *     0 - success
 *     otherwise - return MUGGLE_EVENT_ERROR and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_event_signal_wakeup(muggle_event_signal_t *ev_signal);

/**
 * @brief clearup event signal
 *
 * @param ev_signal  event signal
 *
 * @return 
 * number of times be wakeup, on failed return MUGGLE_EVENT_ERROR and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_event_signal_clearup(muggle_event_signal_t *ev_signal);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_SIGNAL_H_ */
