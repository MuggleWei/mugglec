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
#include "muggle/c/event/event_fd.h"

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

#if MUGGLE_PLATFORM_WINDOWS

#define MUGGLE_EVENT_ERROR SOCKET_ERROR

#define MUGGLE_SYS_ERRNO_INTR       WSAEINTR
#define MUGGLE_SYS_ERRNO_WOULDBLOCK WSAEWOULDBLOCK
#define MUGGLE_SYS_ERROR_AGAIN      WSAEWOULDBLOCK

#else

#define MUGGLE_EVENT_ERROR (-1)

#define MUGGLE_SYS_ERRNO_INTR       EINTR
#define MUGGLE_SYS_ERRNO_WOULDBLOCK EWOULDBLOCK
#define MUGGLE_SYS_ERROR_AGAIN      EAGAIN

#endif

EXTERN_C_BEGIN

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

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_H_ */
