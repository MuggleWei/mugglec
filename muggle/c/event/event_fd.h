/******************************************************************************
 *  @file         event_fd.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-06
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event fd
 *****************************************************************************/

#ifndef MUGGLE_C_EVENT_FD_H_
#define MUGGLE_C_EVENT_FD_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

#include <WinSock2.h>

typedef SOCKET muggle_event_fd;

#define MUGGLE_INVALID_EVENT_FD INVALID_SOCKET

#define MUGGLE_EVENT_FD_SHUT_RD     SD_RECEIVE
#define MUGGLE_EVENT_FD_SHUT_WR     SD_SEND
#define MUGGLE_EVENT_FD_SHUT_RDWR   SD_BOTH

#else

#include <sys/socket.h>

typedef int muggle_event_fd;

#define MUGGLE_INVALID_EVENT_FD (-1)

#define MUGGLE_EVENT_FD_SHUT_RD     SHUT_RD
#define MUGGLE_EVENT_FD_SHUT_WR     SHUT_WR
#define MUGGLE_EVENT_FD_SHUT_RDWR   SHUT_RDWR

#endif

/**
 * @brief close event fd
 *
 * @param fd  event file descriptor
 *
 * @return 
 *     - returns 0 on success
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_ev_fd_close(muggle_event_fd fd);

/**
 * @brief shutdown event fd
 *
 * @param fd   event file descriptor
 * @param how  MUGGLE_EVENT_FD_SHUT_*
 *
 * @return 
 *     - returns 0 on success
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_ev_fd_shutdown(muggle_event_fd fd, int how);

/**
 * @brief set event fd block or non-block
 *
 * @param fd  event file descriptor
 * @param on  if 0, set block, otherwise set non-block
 *
 * @return 
 *     - returns 0 on success
 *     - on error
 */
MUGGLE_C_EXPORT
int muggle_ev_fd_set_nonblock(muggle_event_fd fd, int on);

/**
 * @brief event fd read
 *
 * @param fd   event file descriptor
 * @param buf  buffer used to store receive bytes
 * @param len  size of buffer
 *
 * @return 
 *     - on success, return the number of bytes received, 0 indicates end of event fd
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_ev_fd_read(muggle_event_fd fd, void *buf, size_t len);

/**
 * @brief event fd write
 *
 * @param fd   event file descriptor
 * @param buf  buffer used to store receive bytes
 * @param len  size of buffer
 *
 * @return 
 *     - on success, return the number of bytes written
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_ev_fd_write(muggle_event_fd fd, void *buf, size_t len);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_FD_H_ */
