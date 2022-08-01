/******************************************************************************
 *  @file         event.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-07-31
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event
 *****************************************************************************/

#ifndef MUGGLE_C_EVENT_H_
#define MUGGLE_C_EVENT_H_

#include "muggle/c/base/macro.h"

#if MUGGLE_PLATFORM_WINDOWS

// NOTE: if without WIN32_LEAN_AND_MEAN defined, must
// include winsock2.h before windows.h
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else

#include <stddef.h>
#include <errno.h>

#endif

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

typedef SOCKET muggle_event_fd;

#define MUGGLE_EVENT_ERROR SOCKET_ERROR
#define MUGGLE_INVALID_EVENT_FD INVALID_SOCKET

#define MUGGLE_EVENT_FD_SHUT_RD     SD_RECEIVE
#define MUGGLE_EVENT_FD_SHUT_WR     SD_SEND
#define MUGGLE_EVENT_FD_SHUT_RDWR   SD_BOTH

#define MUGGLE_SYS_ERRNO_INTR       WSAEINTR
#define MUGGLE_SYS_ERRNO_WOULDBLOCK WSAEWOULDBLOCK
#define MUGGLE_SYS_ERROR_AGAIN      WSAEWOULDBLOCK

#else

typedef int muggle_event_fd;

#define MUGGLE_EVENT_ERROR (-1)
#define MUGGLE_INVALID_EVENT_FD (-1)

#define MUGGLE_EVENT_FD_SHUT_RD     SHUT_RD
#define MUGGLE_EVENT_FD_SHUT_WR     SHUT_WR
#define MUGGLE_EVENT_FD_SHUT_RDWR   SHUT_RDWR

#define MUGGLE_SYS_ERRNO_INTR       EINTR
#define MUGGLE_SYS_ERRNO_WOULDBLOCK EWOULDBLOCK
#define MUGGLE_SYS_ERROR_AGAIN      EAGAIN

#endif

#define MUGGLE_EVENT_LAST_ERRNO muggle_event_lasterror()

/**
 * @brief initialize event library
 *
 * @return
 *     0 - success
 *     otherwise - failed
 *
 * @note
 * call this function before use event function
 */
MUGGLE_C_EXPORT
int muggle_event_lib_init();

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
int muggle_event_close(muggle_event_fd fd);

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
int muggle_event_shutdown(muggle_event_fd fd, int how);

/**
 * @brief event function last error no
 *
 * @return error no
 */
MUGGLE_C_EXPORT
int muggle_event_lasterror();

/**
 * @brief get string describing MUGGLE_EVENT_LAST_ERRNO
 *
 * @param errnum   get from MUGGLE_EVENT_LAST_ERRNO
 * @param buf      buffer that stores error string
 * @param bufsize  size of buffer
 *
 * @return returns 0 on success
 */
MUGGLE_C_EXPORT
int muggle_event_strerror(int errnum, char *buf, size_t bufsize);

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
int muggle_event_set_nonblock(muggle_event_fd fd, int on);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_H_ */
