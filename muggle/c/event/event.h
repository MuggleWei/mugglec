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

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

// NOTE: if without WIN32_LEAN_AND_MEAN defined, must
// include winsock2.h before windows.h
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

typedef SOCKET muggle_event_fd;

#define MUGGLE_INVALID_EVENT_FD INVALID_SOCKET

#define MUGGLE_EVENT_FD_SHUT_RD     SD_RECEIVE
#define MUGGLE_EVENT_FD_SHUT_WR     SD_SEND
#define MUGGLE_EVENT_FD_SHUT_RDWR   SD_BOTH

#define MUGGLE_SYS_ERRNO_INTR       WSAEINTR
#define MUGGLE_SYS_ERRNO_WOULDBLOCK WSAEWOULDBLOCK

#else

typedef int muggle_event_fd;

#define MUGGLE_INVALID_EVENT_FD (-1)

#define MUGGLE_EVENT_FD_SHUT_RD     SHUT_RD
#define MUGGLE_EVENT_FD_SHUT_WR     SHUT_WR
#define MUGGLE_EVENT_FD_SHUT_RDWR   SHUT_RDWR

#define MUGGLE_SYS_ERRNO_INTR       EINTR
#define MUGGLE_SYS_ERRNO_WOULDBLOCK EWOULDBLOCK

#endif

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_H_ */
